import argparse
import re
import xml.etree.cElementTree as et
import codegen_utils as code_gen
import json
import os
import sys

CREATION_JSON = "vtest_protocol_objects.json"

INDENT_SIZE = 3
INDENT = INDENT_SIZE * " "
EOL = "\n"
OPEN_SCOPE = "{" + EOL
CLOSE_SCOPE = "}" + EOL
DEFAULT_TYPENAME = 'uint32_t'

def camel_to_snake(name):
    return ''.join([ ("_" + c.lower() if c.isupper() else c) for c in name ])

def generate_prototype(spec):

    function_name = "vtest_" + spec.name
    args = []

    args.append("uint32_t length_dw")
    
    args = ',\n\t'.join(args)
    return 'int {}({});'.format(function_name, args)

def generate_structs(protocol):
    indent = " " * INDENT_SIZE;
    structs = []

    for name in protocol['chunks']:
        chunk = protocol['chunks'][name]
        code = 'struct payload_{0}_{1} '.format(protocol['function'], name)
        code += OPEN_SCOPE

        if chunk['name'] == 'intro':
            code += '{}uint32_t handle;{}'.format(indent, EOL)

        for field in chunk['content']:
            if chunk['name'] == 'intro' and field['name'] == 'handle':
                print("ERROR: 'handle' field in 'intro' chunk is reserved.")
                exit(1)

            typename = DEFAULT_TYPENAME
            if 'type' in field:
                typename = field['type']
            code += '{0}{1} {2};{3}'.format(indent, typename, field['name'], EOL)
            
        code += "};" + EOL

        structs.append(code)

    return structs;

def generate_code_init(protocol):
    indent = " " * INDENT_SIZE
    code = []

    code.append("int res;")
    code.append("uint32_t handle;");
    code.append("struct vtest_result result;")
    code.append("{} vk_info;".format(protocol['input']));

    for name in protocol['chunks']:
        c = protocol['chunks'][name]
        code.append('{0} {1};'.format(c['typename'], name))

    code = [ indent + c for c in code]
    return code

def generate_code_intro_chunk(chunk):
    code = generate_code_simple_chunk(chunk)
    code.insert(0, 'handle = {}.handle;'.format(chunk['name']))

    return code

def generate_code_simple_chunk(chunk):
    code = []

    code.append('res = vtest_block_read(renderer.in_fd, &{0}, sizeof({0}));'.format(
                chunk['name']))
    code.append('CHECK_IO_RESULT(res, sizeof({}));'.format(chunk['name']))
    code.append("")

    for field in chunk['content']:
        code.append('vk_info.{1} = {0}.{1};'.format(chunk['name'], field['name']))

    return code

def get_chunk_dependencies(protocol, chunk):
    deps = [ ]

    parent_node = chunk
    while parent_node['parent'] != None:
        node = protocol['chunks'][parent_node['parent']]
        deps.insert(0, (node, parent_node))
        parent_node = node

    return deps

def generate_code_nested_chunk(indent_level, protocol, chunk):
    indent = " " * INDENT_SIZE * indent_level
    code = [ "" ]

    parent = protocol['chunks'][chunk['parent']]
    dependencies = get_chunk_dependencies(protocol, chunk)

    loop_indent = indent
    iterator = chr(ord('i') + indent_level)
    size_var = '{}.{}'.format(chunk['parent'], chunk['count'])
    size_var = 'vk_info.'
    to_close = 0

    # opening scopes
    for deps in dependencies:
        index_var = size_var + '{}'.format(deps[1]['count'])
        code.append('{0}for (uint32_t {1} = 0; {1} < {2}; {1}++) {{'
                    .format(loop_indent, iterator, index_var))
        loop_indent += INDENT

        size_var += '{}[{}].'.format(deps[1]['name'], iterator)
        iterator = chr(ord(iterator) + 1)
    
    # assignations

    code.append('{0}res = vtest_block_read(renderer.in_fd, &{1}, sizeof({1}));'
                .format(loop_indent, chunk['name']))
    code.append('{}CHECK_IO_RESULT(res, sizeof({}));'
                .format(loop_indent, chunk['name']))
    code.append("")

    for field in chunk['content']:
        code.append('{0}{1}{2} = {3}.{2};'.format(
                    loop_indent, size_var, field['name'], chunk['name']))



    # Closing scopes
    while len(loop_indent) > len(indent):
        loop_indent = loop_indent[:-INDENT_SIZE]
        code += [ loop_indent + "}" ]

    code = [ indent + c for c in code]
    return code

def generate_code_send_chunks(protocol):
    indent = " " * INDENT_SIZE
    code = [ "" ]

    for name in protocol['chunks']:
        chunk = protocol['chunks'][name]

        if chunk['name'] == 'intro' and chunk['parent'] != None:
            print("ERROR: intro chunk must be a root.")
            exit(1)

        if chunk['name'] == 'intro':
            code += generate_code_intro_chunk(chunk)
        elif chunk['parent'] == None:
            code += generate_code_simple_chunk(chunk)
        else:
            code += generate_code_nested_chunk(0, protocol, chunk)


    code = [ indent + c for c in code]
    return code

def generate_code_write_result(protocol):
    indent = " " * INDENT_SIZE
    code = [ "" ]

    code.append('result.error_code = {}('.format(protocol['vgl_function']))
    code.append('{}handle, &vk_info, &result.result);'.format(indent))
    code.append("")
    code.append('res = vtest_block_write(renderer.out_fd, &result, sizeof(result));')
    code.append('CHECK_IO_RESULT(res, sizeof(result));')
    code.append('*output = result.result;')
    code.append('RETURN(result.error_code);')

    code = [ indent + c for c in code]
    return code
    

def generate_function(spec, protocol):
    code = []

    code.append(generate_prototype(spec).replace(";", ""))
    code.append(OPEN_SCOPE)

    code += generate_code_init(protocol)
    code += generate_code_send_chunks(protocol)
    code += generate_code_write_result(protocol)

    code.append(CLOSE_SCOPE)

    code = [ c.rstrip() for c in code ]
    return EOL.join(code)

def cook_entry(protocol):

    for name in protocol['chunks']:
        entry = protocol['chunks'][name]

        entry['name'] = name

        s_name = 'payload_{0}_{1}'.format(protocol['function'], name)
        entry['typename'] = 'struct ' + s_name

HEADER_PROLOG = '           \n\
#pragma once                \n\
                            \n\
#include <vulkan/vulkan.h>  \n\
'

BODY_PROLOG = '             \n\
#include <string.h>         \n\
#include <vulkan/vulkan.h>  \n\
                            \n\
#include "common/macros.h" \n\
#include "virgl_vtest.h" \n\
#include "vtest_protocol.h" \n\
#include "vtest_objects.h"  \n\
'

def generate_code(to_generate, vk_functions):
    prototype_declarations = []
    struct_declarations = []
    function_implems = []

    for entry in to_generate:
        if entry['function'] not in vk_functions:
            print("unknown vk function %s" % entry['function'])
            return (-1, None, None)

        # Camel to snake case
        spec = vk_functions[entry['function']]
        spec.name = camel_to_snake(spec.name).replace("vk_", "")
        entry['function'] = camel_to_snake(entry['function']).replace("vk_", "")
        
        cook_entry(entry)

        prototype_declarations.append(generate_prototype(spec))
        struct_declarations += generate_structs(entry);
        function_implems.append(generate_function(spec, entry))

    header = HEADER_PROLOG
    header += EOL
    header += EOL.join(prototype_declarations)
    header += EOL + EOL
    header += EOL.join(struct_declarations)

    body = BODY_PROLOG
    body += EOL
    body += (EOL * 2).join(function_implems)
    return (0, body, header)

def main():
    parser = argparse.ArgumentParser()
    parser.add_argument('--outdir', help='Where to write the files.', required=True)
    parser.add_argument('--xml', help='Vulkan API XML file.', required=True)
    args = parser.parse_args()

    script_path = os.path.dirname(sys.argv[0])
    to_generate = []
    vk_functions = {}

    with open(os.path.join(script_path, CREATION_JSON)) as f:
        to_generate += json.loads(f.read())

    vk_functions = code_gen.get_entrypoints(et.parse(args.xml))
            
    body_code = ""
    header_code = ""

    err, body_code, header_code = generate_code(to_generate, vk_functions)
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
