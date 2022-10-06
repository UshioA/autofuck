# autofuck
small script executer using `auto' as the only keyword

auto:: -> push
::auto -> pop
auto<A> -> load A
A -> (B)\*E\* | empty
B -> auto(::auto){, 8}
E -> '('A')''
auto-> -> add
->auto -> mul
auto-< -> minus
-<auto -> div
{program} -> while reg do {program}
