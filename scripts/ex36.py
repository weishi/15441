from sys import exit

def main():
    print "What is 1+1"

    next = raw_input("> ")
    if "2" in next:
        do_A()
    else:
        print "Fail"


def do_A():
    counter=1;
    while True:
        print "What is "+str(counter)+"+"+str(counter)
        next = raw_input("> ")
        
        if next == str(counter*2):
            print "More"
            counter=counter+1
        else:
            print "Fail"
            break

main()
