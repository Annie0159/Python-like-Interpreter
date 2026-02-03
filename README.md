# Python-like Interpreter (C)

## Overview
This project is a simple **command-line interpreter written in C** that mimics core behaviors of the **Python 3** interpreter. It provides an interactive environment that supports **dynamic typing**, **variable assignment**, **expression evaluation**, and **nested list** handling. The interpreter successfully passes all provided test cases, including complex inputs involving **floating-point arithmetic** and **lists inside lists**.

## Features
- **Dynamic Types (5 supported)**
  - Long integers
  - Double-precision floating-point numbers
  - Characters
  - Strings
  - Lists (including nested lists)

- **Variable Assignment**
  - Recognizes `=` to assign values or computed expression results to variables
  - Stores variables by name using a symbol table

- **Arithmetic Expressions**
  - Supports basic operations:
    - `+` addition
    - `-` subtraction
    - `*` multiplication
    - `/` division
  - Evaluates expressions such as `a + b` before storing results

- **Printing**
  - Supports `print(...)` to display variable values
  - Can print lists and all list elements (including nested structures)

- **List Manipulation**
  - Creates lists and supports appending items to the end of a list
  - Handles nested list structures correctly

## How It Works (High-Level)
1. The interpreter reads user input from the command line.
2. It determines intent based on syntax:
   - If input contains `=`, it performs **assignment** (value or expression result).
   - If input contains `print(...)`, it **displays** the requested variable/value.
3. Values are stored in a **symbol table** implemented as a linked list of variables.
4. Each variable holds a flexible value type capable of representing numbers, text, or lists.

## Internal Design
- **Symbol Table:** Linked list storing `(variable_name, value)`
- **Value System:** A flexible value representation that can store:
  - integer / float / char / string / list
- **List Representation:** Supports lists containing mixed types, including other lists

## Results
The interpreter successfully handles the required commands and operations and **passes all provided example tests**, including cases with **nested lists** and **floating-point arithmetic**.

## Skills Demonstrated
- Parsing and interpreting user input
- Dynamic memory management in C
- Data structure design (linked lists, nested structures)
- Implementing an interactive REPL-like environment
- Building a simplified language runtime from scratch
