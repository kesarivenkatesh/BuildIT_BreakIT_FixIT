# Sample Input and Output
## Input - command line arguments
## Output - text


g++ -std=c++11 logappend.cpp -o logappend

./logappend -T 1 -K secret -A -E Fred log1
./logappend -T 2 -K secret -A -G Jill log1
./logappend -T 3 -K secret -A -E Fred -R 1 log1
./logappend -T 4 -K secret -A -G Jill -R 1 log1

g++ -std=c++11 logread.cpp -o logread 

./logread -K secret -S log1

Fred
Jill
1: Fred,Jill



./logappend -T 5 -K secret -L -E Fred -R 1 log1
./logappend -T 6 -K secret -A -E Fred -R 2 log1
./logappend -T 7 -K secret -L -E Fred -R 2 log1
./logappend -T 8 -K secret -A -E Fred -R 3 log1
./logappend -T 9 -K secret -L -E Fred -R 3 log1
./logappend -T 10 -K secret -A -E Fred -R 1 log1

./logread -K secret -R -E Fred log1

1,2,3,1 