# Authors: Landon Carderara, Nico Relle
## Date: 2/19/26

## Contributions:
Both collaberators made a verion of the techshell both labeled with their names and after deliberation we decided to have "final_techshell_Landon.c" be our final submission.

We chose this because "final_techshell_Landon.c" had better commentation as well as better handling of user input to allow for more flexability for the user to make mistakes when writing commands.

"techshell-Nico.c" worked as inteneded but wasn't able to properly implement quatation use to better allow freedom for the user but the code would still allow the user to complete the processes in the way they wished
  but had to complete the syntax and a specific non-flexable manner.

## Description:
This code makes a shell inside the command prompt that allows users to edit files and folders and run commands to accomplish tasks.

## Clone Instructions:
1. First naviagate to the location you wish to store the techshell file and hold the executable file.
2. Once there ensure you have git installed by
```sh
git -v
```
a. if not downloaded run 
  ```sh
  git install
  ```
3.  then clone the repo in your folder
```sh
git clone https://github.com/nlr8116/techshell-Project.git
```
4. once cloned go into the repo directory and compile the techshell file
```sh
cd "techshell-project" ; gcc "final_techshell_Landon.c" -o techshell
```
5. Now that you've compiled the techsell all ypu have to do is now run the file with this line and you will be able to naviagte and execute commands to acoplish many tasks
```sh
./techshell
```
