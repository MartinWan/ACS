README
  ACS is a simulation tool for simulating airline check ins.
  Developed for CSC 360 - Operating Systems

INSTALLATION
  Compile the code by typing `make` into the command line

RUNNING
  After installation, type ./ACS <input file> to 
  start the program with an input file specifying customers to be simulated.
  The input file must obey the input file format specified below.
  
INPUT FILE FORMAT
 The first line contains the total number of customers that will be simulated.
 After that, each line contains the information about a single customer, such that:
  1. The first character specifies the unique ID of customers
  2. A colon(:) immediately follows the unique number of the customer.
  3. Immediately following is an integer that indicates the arrival time of the customer
  4. A comma(,) immediately follows the previous number.
  5. Immediately following is an integer that indicates the service time of the customer.
  6. A newline (\n) ends a line.
  
  Sample Input:
    7
    1:2,60
    2:4,70
    3:5,50
    4:7,30
    5:7,40
    6:8,50
    7:10,30
