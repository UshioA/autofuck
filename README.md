# autofuck
small script executer using `auto` as the only keyword

```
auto:: -> push  
::auto -> pop  
auto<A> -> load A  
A -> (B)*E* | empty  
B -> auto(::auto){, 8}  
E -> '('A')'
auto-> -> add  
->auto -> mul  
auto-< -> minus  
-<auto -> div  
{program} -> while reg do {program}  
```
## Usage

compile the source yourself, and in shell you type
```bash
./autofuck [any parameter]
```
to start program. If there is one or more parameter the program will show debug output.

You put string into the program and it will execute it. Note that autofuck dosen't accept whitespaces. 

Example program:
```
{auto<auto::auto>;auto::auto<>;auto<auto::auto::auto::auto(auto::auto::auto::auto::auto::auto::auto::auto>auto::auto-<::auto}
```
This program repeats your input char until you enter an `'0'`.