

ten_things = "Apples Oranges Crows Telephone Light Sugar"

stuff = ten_things.split(' ')
more_stuff = ["Day", "Night", "Song", "Frisbee", "Corn", "Banana", "Girl", "Boy"]

while len(stuff) != 10:
    next_one = more_stuff.pop()
    print "Adding: ", next_one
    stuff.append(next_one)
    print "%d items now." % len(stuff)

print stuff

print stuff[1]
print stuff[-1]
print stuff.pop()
print ' '.join(stuff) 
print '#'.join(stuff[3:5])
