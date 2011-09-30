cars = 100
space_in_a_car = 4.0
drivers = 30
passengers = 90
cars_not_driven = cars - drivers
cars_driven = drivers
carpool_capacity = cars_driven * space_in_a_car
average_passengers_per_car = passengers / cars_driven


print "There are", cars, "There are only", drivers, 
print "There will be", cars_not_driven,"We can transport", carpool_capacity,
print "We have", passengers,"We need to put about", average_passengers_per_car,
