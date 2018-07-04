import argparse
import re
import xml.etree.cElementTree as et
import codegen_utils as code_gen
import json
import os
import sys

CREATION_JSON = "vtest_protocol_objects.json"
DEFAULT_TYPENAME = "uint32_t"
INDENT_CHAR=' '
INDENT_SIZE=3

# Utils
def indent(size, line):
    if type(line) is list:
        return [ (INDENT_CHAR * INDENT_SIZE * size) + l for l in line ];
    return (INDENT_CHAR * INDENT_SIZE * size) + line;

def camel_to_snake(name):
    return ''.join([ ("_" + c.lower() if c.isupper() else c) for c in name ])

def spec_get_field_type(spec, struct, field):
    if struct not in spec:
        raise Exception("Unknown typename {}".format(struct))

    st = spec[struct]

    print(struct, field)

    for m in st['members']:
        if m.name == field:
            print(m.type)
            return m.type
    
    raise Exception("Unknown field {}".format(field))
    return ""

def generate_structures(protocol):
    fname = camel_to_snake(protocol['vk_function'])[3:]
    structs = []

    for name in protocol['chunks']:
        chunk = protocol['chunks'][name]
        code = []

        code.append('struct payload_{0}_{1} '.format(fname, name) + "{")

        if name == 'intro':
            code.append(indent(1, 'uint32_t handle;'))

        for field in chunk['content']:
            if name == 'intro' and field['name'] == 'handle':
                raise Exception("'handle' field in 'intro' chunk is reserved.")

            typename = DEFAULT_TYPENAME
            if 'type' in field:
                typename = field['type']

            code.append(indent(1, '{} {};'.format(typename, field['name'])))
            
        code.append("};")
        code.append("")

        structs.append("\n".join(code))

    return structs;

# Structures

class Component:
    def __init__(self, spec, name, infos, typename, parent = None):
        self.spec = spec
        self.name = name
        self.infos = infos

        self.components = {}
        self.parent = parent
        self.input_typename = typename

        if parent == None:
            self.output_typename = infos['info_type']
        else:
            self.output_typename = spec_get_field_type(self.spec,
                                                       self.parent.output_typename,
                                                       self.name)

        if parent == None and typename == None:
            raise Exception("parent and typename cannot be both NULL")

    def AddSubcomponent(self, name, infos):
        if self.name == infos['parent']:
            typename = self.input_typename[:-len(self.name)] + name
            self.components[name] = type(self)(self.spec, name, infos, typename, self)
            return True

        for key in self.components:
            if self.components[key].AddSubcomponent(name, infos):
                return True

        return False

    def GetIterator(self):
        if self.parent == None:
            return "i"
        return chr(ord(self.parent.GetIterator()) + 1)

    def GetSizeVar(self):
        if self.parent.parent == None:
            return '{}.{}'.format(self.parent.name, self.infos['count'])

        return '{}[{}].{}'.format(self.parent.name, self.parent.parent.GetIterator(),
            self.infos['count'])

    def GetDeclarations(self):
        output = []
        varname = self.name

        if self.parent == None:
            output.append("{} {};".format(self.input_typename, varname))
            return output

        output.append("{} tmp_{};".format(self.input_typename, varname))

        if self.parent.parent == None:
            prefix = "vk_info"
        else:
            prefix = self.parent.name
        typename = self.output_typename
        output.append("{} *{} = NULL;".format(typename, varname))
        output.append("{0} = alloca(sizeof(*{0}) * {1}.{2});".format(
            varname, prefix, self.infos['count']))

        return output

    def GetTransfertCode(self, prefix = None):
        raise Exception("Component class should not be used directly.")
        return []

class VtestRecvComponent(Component):
    def __init__(self, spec, name, infos, parent = None, typename = None):
        super().__init__(spec, name, infos, parent, typename)

        self.output = infos['handle_type']

    def GetReadCode(self, prefix = None):
        if prefix == None:
            storage = self.name
        else:
            storage = "tmp_" + self.name

        return [
            "res = vtest_block_read(renderer.in_fd, &{0}, sizeof({0}));".format(
                storage),
            "CHECK_IO_RESULT(res, sizeof({}));".format(storage)
        ]

    def GetAttributeCopyCode(self, prefix = None):
        if not 'content' in self.infos:
            if prefix == None:
                return [ "vk_info = {0};".format(self.name) ]
            else:
                return [ "{0}[{1}] = tmp_{0};".format(
                        self.name, self.parent.GetIterator()) ]

        output = []
        if prefix == None:
            for f in self.infos['content']:
                output.append("vk_info.{1} = {0}.{1};".format(
                    self.name, f['name']))

        else:
            for f in self.infos['content']:
                output.append("{0}[{1}].{2} = tmp_{0}.{2};".format(
                    self.name, self.parent.GetIterator(), f['name']))
        return output

    def GetTransfertCode(self, prefix = None):
        output = []

        output += self.GetReadCode(prefix)
        output += self.GetAttributeCopyCode(prefix)

        if len(self.components) == 0:
            return output
        output.append("")

        for key in self.components:
            component = self.components[key]
            output += component.GetDeclarations()
            output.append("")

            output.append("for (uint32_t {0} = 0; {0} < {1}; {0}++)".format(
                self.GetIterator(), component.GetSizeVar()) + " {")

            output += indent(1, component.GetTransfertCode(self.name))
            output.append("}")

            output.append("")

            if prefix == None:
                output.append("vk_info.{0} = {0};".format(key))
            else:
                output.append("{0}[{1}].{2} = {2};".format(
                    self.name, self.parent.GetIterator(), key))
        return output

class CodeBlock:
    def __init__(self, content = None):
        if content == None:
            content = []

        self.content = content

    def GetPrototype(self):
        return []

    def GetBody(self):
        output = []
        
        output.append("{")

        for l in self.content:
            output.append(indent(1, line))

        output.append("}")

        return output

    def __str__(self):
        return "\n".join(self.GetContent())

class FunctionBlock(CodeBlock):
    def __init__(self, name, return_value, params, infos):
        super().__init__()
        self.return_value = return_value
        self.name = name
        self.args = params
        self.infos = infos

    def SetComponents(self, components):
        self.components = components

    def GetPrototype(self):
        output = []
        output.append(self.return_value)
        output.append('{}('.format(self.name))

        parameters = []
        for arg in self.args:
            parameters.append(indent(1, str(arg)))
        output.append(",\n".join(parameters))

        output.append(")")
        return output

    def GetProlog(self):
        return [
            "TRACE_IN();",
            "UNUSED_PARAMETER(length_dw);"
        ]

    def GetStandardDeclarations(self):
        return []

    def GetSpecificDeclarations(self):
        output = []

        # We only need the root declarations
        for c in self.components:
            output += self.components[c].GetDeclarations()

        return output

    def GetTransfertCode(self):
        output = []
        for c in self.components:
            output += self.components[c].GetTransfertCode()
        return output

    def GetEpilog(self):
        return []

    def GetBody(self):
        output = []
        output.append("{")

        output += indent(1, self.GetStandardDeclarations())
        output.append("")
        output += indent(1, self.GetSpecificDeclarations())
        output.append("")
        output += indent(1, self.GetTransfertCode())
        output.append("")
        output += indent(1, self.GetProlog())

        output.append("}")
        return output

class VtestRecvFunction(FunctionBlock):
    def GetPrototype(self):
        output = []
        output.append("int")
        output.append("vtest_{}(uint32_t length_dw)".format(
            camel_to_snake(self.name)))

        return output

    def GetStandardDeclarations(self):
        output = []

        # Used by all IO functions
        output.append("int res;")

        # The parent handle
        output.append("uint32_t handle;")

        # The output struct
        output.append("struct vtest_result result;")
        output.append("{} vk_info;".format(self.infos['info_type']))

        return output

    def GetProlog(self):
        output = []

        output.append("result.error_code = {}(handle, &vk_info, &result.result);".format(
            self.infos['vgl_function']))
        output.append("res = vtest_block_write(renderer.out_fd, &result, sizeof(result));")
        output.append("CHECK_IO_RESULT(res, sizeof(result));")

        output.append("")
        output.append("TRACE_OUT();")
        output.append("RETURN(0);")

        return output


def prepare_protocol_components(spec, protocol):
    to_generate = []

    for entry in protocol:
        components = {}
        fname = camel_to_snake(entry['vk_function'])
        fname = fname.replace("vk_", "")

        for name in entry['chunks']:
            chunk = entry['chunks'][name]
            chunk['info_type'] = entry['info_type']
            chunk['handle_type'] = entry['handle_type']

            if chunk['parent'] == None:
                typename = "struct payload_{}_{}".format(fname, name)
                components[name] = VtestRecvComponent(spec, name, chunk, typename)
                continue
            
            added = False
            for key in components:
                if components[key].AddSubcomponent(name, chunk):
                    added = True
                    break
            
            if not added:
                raise KeyError("Unknown parent chunk {}".format(chunk['parent']))

        to_generate.append((entry, components))

    return to_generate

def generate_header_prolog():
    header = []
    header.append("#ifndef VTEST_VK_PROTOCOL")
    header.append("#define VTEST_VK_PROTOCOL")
    header.append("")

    return header

def generate_body_prolog():
    body = []

    body.append("/*")
    body.append(" This file has been generated. Please do not modify it.")
    body.append("*/")

    body.append("#include <stdio.h>")
    body.append("#include <stdlib.h>")
    body.append("#include <string.h>")
    body.append("#include <vulkan/vulkan.h>")
    body.append("")
    body.append('#include "virglrenderer_vulkan.h"')
    body.append('#include "util/macros.h"')
    body.append('#include "vtest.h"')
    body.append('#include "vtest_protocol.h"')
    body.append('#include "vtest_vk.h"')
    body.append('#include "vtest_vk_objects.h"')
    body.append("")
    body.append("extern struct vtest_renderer renderer;")
    body.append("")

    return body

def generate_header_epilog():
    return [ "", "#endif" ]

def generate_body_epilog():
    return []

def postprocess(code):
    code = [ l.rstrip() for l in code ]
    return "\n".join(code)

def generate_header(protocol, spec, to_generate):
    header = []

    header += generate_header_prolog()

    for elt in protocol:
        header += generate_structures(elt)
    header.append("")

    for task in to_generate:
        fname = task[0]['vk_function']
        function = VtestRecvFunction(fname,
                                 spec[fname].ret_value,
                                 spec[fname].params,
                                 task[0])
        function.SetComponents(task[1])
        header += function.GetPrototype()
        header[-1] += ";"

    header += generate_header_epilog()

    return header

def generate_body(protocol, spec, to_generate):
    body = []
    body += generate_body_prolog()

    for task in to_generate:
        fname = task[0]['vk_function']
        function = VtestRecvFunction(fname,
                                 spec[fname].ret_value,
                                 spec[fname].params,
                                 task[0])
        function.SetComponents(task[1])
        body += function.GetPrototype()
        body += function.GetBody()
        body.append("")

    body += generate_body_epilog()
    return body

def generate_code(protocol, spec):
    to_generate = prepare_protocol_components(spec, protocol)

    header = generate_header(protocol, spec, to_generate)
    body = generate_body(protocol, spec, to_generate)

    return (
        0,
        postprocess(body),
        postprocess(header)
    )

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--outdir', help='Where to write the files.', required=True)
    parser.add_argument('--xml', help='Vulkan API XML file.', required=True)
    args = parser.parse_args()

    script_path = os.path.dirname(sys.argv[0])
    protocol = []
    spec = {}

    with open(os.path.join(script_path, CREATION_JSON)) as f:
        protocol = json.loads(f.read())

    spec = code_gen.load_spec(et.parse(args.xml))
            
    body_code = ""
    header_code = ""

    err, body_code, header_code = generate_code(protocol, spec)
    if err != 0:
        return err;

    outputs = [
        ("vtest_vk_objects.h", header_code),
        ("vtest_vk_objects.c", body_code),
    ]

    for task in outputs:
        path = os.path.join(args.outdir, task[0])
        with open(path, "w") as f:
            f.write(task[1])
            print("{} generated.".format(task[0]))

if __name__ == '__main__':
    exit(main())
