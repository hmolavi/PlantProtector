# Plant Protector

# Project Naming Convention

This project follows this naming convention

| Item                     | Naming Style          |
| ------------------------ | --------------------- |
| Global Variables         | g_snake_case          |
| Local Variables          | snake_case            |
| Global Functions         | Module_PascalCase     |
| Private/Static Functions | snake_case            |
| Structs                  | PascalCase            |
| Struct Members           | snake_case            |
| Enum                     | ETitleCase            |
| Enum Members             | ALL_CAPS or camelCase |

Ports will be static to files that use them, might have just one file responsible for reading of all sensors.


WIP: Parameters to save to sd card
| timestamp  | raw_moisture | filtered_moisture | temp | humidity | watering | pump_dur | season | drop_rate | threshold |
| ---------- | ------------ | ----------------- | ---- | -------- | -------- | -------- | ------ | --------- | --------- |
| 1689234567 | 845          | 801               | 22.5 | 45.0     | 1        | 30       | 2      | 1.2       | 300       |
| 1689234570 | 820          | 798               | 22.6 | 44.8     | 0        | 0        | 2      | 1.1       | 300       |

## Personal Notes, delete before launch
Read from right to left  

int const *ptr; // ptr is a pointer to constant int  
const int *ptr // ptr is a pointer to constant int  

int *const ptr; // ptr is a constant pointer to int  


# To clone
Because of the aws sdk import you must also update the submodules that comes with it, you can do it with:
``` bash
git submodule update --init --recursive
```