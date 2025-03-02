import sys
import re

def no_args(resulting_instruction):
    def return_instruction(args_list, labels):
        return resulting_instruction
    
    return return_instruction


def jmp(args_list, labels):
    target_address = labels[args_list[1]]

    return [25, 0, (target_address & 0xFF00) >> 8, target_address & 0x00FF, 21, 0, 0, 0]

def jez(args_list, labels):
    target_address = labels[args_list[1]]

    return [25, 0, (target_address & 0xFF00) >> 8, target_address & 0x00FF, 22, 0, 0, 0]

def jnz(args_list, labels):
    target_address = labels[args_list[1]]

    return [25, 0, (target_address & 0xFF00) >> 8, target_address & 0x00FF, 23, 0, 0, 0]


def lia(args_list, labels):
    immediate = int(args_list[1])
    return [24, 0, (immediate & 0xFF00) >> 8, immediate & 0x00FF]

def lib(args_list, labels):
    immediate = int(args_list[1])
    return [25, 0, (immediate & 0xFF00) >> 8, immediate & 0x00FF]


def lra(args_list, labels):
    identifier = 255

    if args_list[1] == "sp":
        identifier = 0
    elif args_list[1] == "pc":
        identifier = 1
    elif args_list[1] == "cf":
        identifier = 2

    return [29, 0, 0, identifier]

instruction_generators = {
    "hlt" : no_args([0, 0, 0, 0]),

    "add" : no_args([4, 0, 0, 0]),
    "sub" : no_args([5, 0, 0, 0]),
    "mul" : no_args([6, 0, 0, 0]),
    "div" : no_args([7, 0, 0, 0]),

    "ltb" : no_args([8, 0, 0, 0]),
    "bsr" : no_args([9, 0, 0, 0]),
    "bsl" : no_args([10, 0, 0, 0]),
    "mod" : no_args([11, 0, 0, 0]),

    "bio" : no_args([12, 0, 0, 0]),
    "bia" : no_args([13, 0, 0, 0]),
    "bix" : no_args([14, 0, 0, 0]),
    "bin" : no_args([15, 0, 0, 0]),
    
    "blo" : no_args([16, 0, 0, 0]),
    "bla" : no_args([17, 0, 0, 0]),
    "blx" : no_args([18, 0, 0, 0]),
    "bln" : no_args([19, 0, 0, 0]),

    "teq" : no_args([20, 0, 0, 0]),
    "jmp" : jmp,
    "jez" : jez,
    "jnz" : jnz,

    "lia" : lia,
    "lib" : lib,
    "swp" : no_args([26, 0, 0, 0]),
    "lma" : no_args([27, 0, 0, 0]),
    "sma" : no_args([28, 0, 0, 0]),
    "lra" : lra,
    "pop" : no_args([30, 0, 0, 0]),
    "psh" : no_args([31, 0, 0, 0]),
}

def filter_labels(instructions):
    filtered_instructions = []
    labels = dict()

    instruction_index = 0
    for line in instructions:
        if re.match("^[a-zA-Z_-]+[a-zA-Z0-9_-]*:", line[0]) != None:
            labels[line[0][:-1]] = instruction_index
            continue

        filtered_instructions.append(line)
        instruction_index += 4

    return filtered_instructions, labels

if __name__ == "__main__":
    if len(sys.argv) != 3:
        print("Assembler not supplied with input file and output location")
        quit(-1)
    
    with open(sys.argv[1], "r") as in_asm:
        assembly = in_asm.read()
        
        #remove each comment until the newline
        assembly = re.sub("--[^\n]*", "", assembly)

        #break up each line into the operation and its arguments
        instructions = assembly.split("\n")
        instructions = [line.strip() for line in instructions if line.strip(" \t") != ""]
        instructions = [line.split(" ") for line in instructions]
        instructions, labels = filter_labels(instructions)

        #convert each instruction into machine code
        program = [byte for line in instructions for byte in instruction_generators[line[0]](line, labels)]

        with open(sys.argv[2], "wb") as out_bin:
            out_bin.write(bytes(program))