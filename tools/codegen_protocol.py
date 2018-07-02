import argparse
import re
import xml.etree.cElementTree as et
import codegen_utils as code_gen
import json
import os
import sys

CREATION_JSON = "vtest_protocol_objects.json"
INDENT_CHAR=' '
INDENT_SIZE=3

# Utils
def indent(size, line):
    if type(line) is list:
        return [ (INDENT_CHAR * INDENT_SIZE * size) + l for l in line ];
    return (INDENT_CHAR * INDENT_SIZE * size) + line;

def camel_to_snake(name):
    return ''.join([ ("_" + c.lower() if c.isupper() else c) for c in name ])


# Structures

class Component:
    def __init__(self, name, infos):
        self.name = name
        self.info = infos
        self.components = {}

    def add_subcomponent(self, name, infos):
        if self.name == infos['parent']:
            self.components[name] = Component(name, infos)
            return True

        for key in self.components:
            if self.components[key].add_subcomponent(name, infos):
                return True

        return False

    def GetDeclarations(self):
        output = []

        output.append("{} {};".format(self.name, "tmp"))

        return output

    def GetTransfertCode(self):
        output = []

        output.append("{} = {};".format("tmp", 12))

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
    def __init__(self, name, return_value, params):
        super().__init__()
        self.return_value = return_value
        self.name = name
        self.args = params

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

    def GetStandardDeclarations(self):
        return []

    def GetSpecificDeclarations(self):
        output = []

        # We only need the root declarations
        for c in self.components:
            output += indent(1, self.components[c].GetDeclarations())

        return output

    def GetTransfertCode(self):
        output = []
        for c in self.components:
            output += indent(1, self.components[c].GetTransfertCode())
        return output

    def GetProlog(self):
        return []

    def GetBody(self):
        output = []
        output.append("{")

        output += self.GetStandardDeclarations()
        output += self.GetSpecificDeclarations()
        output += self.GetTransfertCode()
        output += self.GetProlog()

        output.append("}")
        return output

class VtestRecvFunction(FunctionBlock):
    def __init__(self, name, return_value, params):
        super().__init__(name, return_value, params)



def prepare_protocol_components(protocol):
    to_generate = []

    for entry in protocol:
        components = {}
        
        for name in entry['chunks']:
            chunk = entry['chunks'][name]

            if chunk['parent'] == None:
                components[name] = Component(name, chunk)
                continue
            
            added = False
            for key in components:
                if components[key].add_subcomponent(name, chunk):
                    added = True
                    break
            
            if not added:
                raise KeyError("Unknown parent chunk {}".format(chunk['parent']))

        to_generate.append((entry, components))

    return to_generate

def generate_code(protocol, vk_functions):
    to_generate = prepare_protocol_components(protocol)
    header = ""
    body = ""

    for task in to_generate:
        fname = task[0]['function']
        function = FunctionBlock(fname,
                                 vk_functions[fname].ret_value,
                                 vk_functions[fname].params)
        function.SetComponents(task[1])

        header += "\n".join(function.GetPrototype())
        header += ";\n\n"

        body += "\n".join(function.GetPrototype()) + "\n"
        body += "\n".join(function.GetBody())
        body += "\n\n"

    return (0, body, header)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--outdir', help='Where to write the files.', required=True)
    parser.add_argument('--xml', help='Vulkan API XML file.', required=True)
    args = parser.parse_args()

    script_path = os.path.dirname(sys.argv[0])
    protocol = []
    vk_functions = {}

    with open(os.path.join(script_path, CREATION_JSON)) as f:
        protocol = json.loads(f.read())

    vk_functions = code_gen.get_entrypoints(et.parse(args.xml))
            
    body_code = ""
    header_code = ""

    err, body_code, header_code = generate_code(protocol, vk_functions)
    if err != 0:
        return err;

    outputs = [
        ("vtest_objects.h", header_code),
        ("vtest_objects.c", body_code),
    ]

    for task in outputs:
        path = os.path.join(args.outdir, task[0])
        with open(path, "w") as f:
            f.write(task[1])
            print("{} generated.".format(task[0]))

if __name__ == '__main__':
    exit(main())
