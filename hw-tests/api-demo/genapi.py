import argparse
import contextlib
import os
import re
import subprocess


def main():
    parser = argparse.ArgumentParser(
        description="Generate the API stubs from a header file."
    )
    parser.add_argument(
        "-H", "--header", required=True, help="The header to base the definitions on."
    )
    parser.add_argument(
        "-c", "--client", required=True, help="The output client-side c source file."
    )
    parser.add_argument(
        "-s", "--server", required=True, help="The output server-side c source file."
    )
    args = parser.parse_args()

    with contextlib.ExitStack() as cx:
        # Run the preprocessor on the header file to get the API definitions.
        #
        # For this, we first need a source to include the header which contains
        # an alternative definition of the `API` macro that marks definitions in
        # a way we can find later on.
        api_src = """\
#define API(id, def) __GENERATE_API $ __GEN_ID_##id $ def $
#include "{header}"
""".format(
            header=os.path.relpath(args.header)
        )

        # Evaluate the preprocessor
        source = subprocess.check_output(
            ["gcc", "-E", "-"], input=api_src.encode()
        ).decode()

        # Parse the header for API definitions
        matcher = re.compile(
            r"__GENERATE_API \$ __GEN_ID_(?P<id>\w+) \$ (?P<decl>.+?)\((?P<args>.*?)\) \$",
            re.DOTALL | re.MULTILINE,
        )

        args_matcher = re.compile(r"(?P<type>\w+(?:\*+|\s+))(?P<name>\w+),")

        # Open output files
        f_client = cx.enter_context(open(args.client, "w"))
        f_server = cx.enter_context(open(args.server, "w"))

        print('#include "{}"\n'.format(
            os.path.basename(args.header)
        ), file=f_client)
        print('#include "{}"\n'.format(
            os.path.basename(args.header)
        ), file=f_server)

        for match in matcher.finditer(source):
            api_id = match.group("id")
            api_decl = match.group("decl")
            api_args = match.group("args")

            api_args_names = []
            api_args_types = []
            api_args_sizes = []

            # Destructure args
            for match in args_matcher.finditer(api_args + ","):
                arg_type = match.group("type").strip()
                arg_name = match.group("name")

                api_args_names.append(arg_name)
                api_args_types.append(arg_type)
                api_args_sizes.append("sizeof({})".format(arg_type))

            print(
                """\
/* Autogenerated stub for {id} */
{cdecl}({cargs}) {{
    const int size = {total_size};
    unsigned char buffer[size];
""".format(
                    id=api_id,
                    cdecl=api_decl,
                    cargs=api_args,
                    total_size=" + ".join(api_args_sizes),
                ),
                file=f_client,
            )

            for i, (arg, ty) in enumerate(zip(api_args_names, api_args_types)):
                print(
                    """    *({type}*)(buffer + {offset}) = {arg};""".format(
                        type=ty,
                        offset=" + ".join(api_args_sizes[:i]) if i > 0 else "0",
                        arg=arg,
                    ),
                    file=f_client,
                )

            print(
                """
    printf("{id}: ");
    for (int i = 0; i < size; i++) {{
        printf("0x%02x ", buffer[i]);
    }}
    printf("\\n");
}}
""".format(
                    id=api_id
                ),
                file=f_client,
            )


if __name__ == "__main__":
    main()
