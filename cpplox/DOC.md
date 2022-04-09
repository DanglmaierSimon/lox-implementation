# Semantics

## Lists

Indexed by [], first index is 0
Created also by []

Index may not be negative
Index out of range == runtime error
Elements can be appended with built in function append() (like in Go, list automatically grows)
Lists can be appended to other lists (appended at last element) (also with built-in function append)
Lists can have mixed entries
Lists are printed by iterating over each value, printing that value, then a comma, then a space
Each expression is evaluated from left to right, in order of declaration, each expression is evaluated before the list is created
2 Lists (List a and List b) are equal, if List a and List b are the same length and if each element in List a is equal to the element at the same index in List B
Lists can contain other lists
The length of a list can be determined by the built-in function len() (like in go)
Precedence?

a = [12, 45, 67] // Create list
a = append(a, 89) // a is now [12, 45, 67]
b = [33]

a = append(a, b) // a is now  [12, 45, 67, 33]

a = [12, "hello", true]
a = [12 == 13, 14 == 14] // a is [false, true]

a = [[1, 2],[4, 4]] 

b = a[0] // b would be [1, 2]

// special case to consider: an array creation expression can immediately be indexed (like in python)

c = [34, 35, 36][0] // c would be 34