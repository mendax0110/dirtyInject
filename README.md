# dirtyInject

**dirtyInject** is a command-line tool designed for injecting DLLs into Windows processes or SO libraries into Linux processes. It supports Windows, Linux and macOS environments and provides a straightforward interface for injecting malicious code (for testing and research purposes only).

## Usage

To use **dirtyInject**, follow these steps:

1. Clone the repository:
   ```bash
   git clone https://github.com/mendax0110/dirtyInject.git

2. Navigate to the project directory:
   ```bash
   cd dirtyInject

3. Create a build directory:
   ```bash
   mkdir build

4. Navigate to the build directory:
   ```bash
    cd build

5. Generate the build files:
    ```bash
    cmake ..

6. Build the project:
    ```bash
    cmake --build .

7. Run the executable:
    ```bash
    dirtyInject.exe <dll_path/so_path/dyLib_path> <process_name>


## Supported Platforms

**dirtyInject** is supported on the following platforms:

- Windows
- Linux
- macOS

