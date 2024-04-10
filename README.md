# Command README

## Description
This is a small assmebler that takes in a file with assembly code and outputs a file with the machine code. with a toy instruction set. The instruction set is as follows:
```
    ADD <reg1> <reg2> <reg3> // reg1 = reg2 + reg3
    SUB <reg1> <reg2> <reg3> // reg1 = reg2 - reg3
    HALT                     // halt the program
```

## Usage
To run the command, follow these steps:

1. Clone the repository: `git clone <repository_url>`
2. Navigate to the project directory: `cd <project_directory>`
3. Run the command: `make`


## Examples
Here are some examples of how to use the command:

- Run the command with default options:
    ```
    ./Assembler <input_file> 
    ```

## License
This command is licensed under the [MIT License](LICENSE).