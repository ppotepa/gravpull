
![image](https://github.com/user-attachments/assets/14a7a03a-a706-45be-b8ef-abe4f9657bc0)
![image](https://github.com/user-attachments/assets/224420e8-99ce-4d7f-87bc-0334c6fd0a23)



# Grav Pull

Grav Pull is a C++ application designed to demonstrate or simulate gravitational pull mechanics. The project is built using Visual Studio and leverages the Allegro library for graphics and other functionalities.

## Requirements

- **Visual Studio 2022** or later
- **C++14** support
- **Allegro 5.2.4.0** or later (included via NuGet packages)
- **Vcpkg NuGet 1.5.0**

## Build Configuration

This project is configured to work **only in `Release x64` mode**. Ensure that you select the correct configuration in Visual Studio before building or running the project.

### Steps to Set the Configuration:
1. Open the project in Visual Studio.
2. Go to the __Configuration Manager__:
   - Navigate to __Build > Configuration Manager__.
   - Set the "Active Solution Configuration" to `Release`.
   - Set the "Active Solution Platform" to `x64`.
3. Save the changes.

## How to Build and Run

1. Clone or download the repository.
2. Open the `grav_pull.sln` file in Visual Studio.
3. Ensure the configuration is set to `Release x64`.
4. Build the solution using the __Build > Build Solution__ option.
5. Run the application using the __Debug > Start Without Debugging__ option.

## Dependencies

The project uses the following dependencies:
- **Allegro 5.2.4.0**: A cross-platform library for handling graphics, input, and more.
- **Vcpkg NuGet 1.5.0**: A package manager for C++ libraries.

These dependencies are managed via NuGet and should be restored automatically when you build the project.

## Notes

- Ensure that all required NuGet packages are restored before building the project. If packages are missing, use the __Restore NuGet Packages__ option in Visual Studio.
- The project is configured to copy necessary Allegro DLLs to the output directory after building.

## License

This project is licensed under the terms specified in the repository. Refer to the `LICENSE` file for more details.
