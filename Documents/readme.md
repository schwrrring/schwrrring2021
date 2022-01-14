###install
#### install git submodules:
1. git submodule init
2. git submodule 

reload CMakeList.txt

###CmakeList.txt

Target properties are defined in one of two scopes: INTERFACE and PRIVATE. 
Private properties are used internally to build the target, while interface properties are used externally by users of the target.
In other words, interface properties model usage requirements, whereas private properties model build requirements of targets.

=> Das heisst eventuell, dass ich ich das interface nicht brauche. Weil meine Anwendung keine Library ist?


## Tests
Tests bekommen ihre eingen CMakeList