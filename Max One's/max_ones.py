import random

def Fitness():
    Elite = 0
    for i in range(len(pop)):
        temp = sum(pop[i])
        if temp > Elite:
            Elite = temp
            Current_elite = i
    # If there is an individual w/ 20 1's, Return True
    if Elite == 20:
        print('Solution Found: Individual ' + str(Current_elite + 1) + ' (Index ' + str(Current_elite) + ') contain`s all 1`s.')
        print(str(Current_elite) + ': ' + str(pop[Current_elite]))
        return True
    # Else appended the current best individual to the new population
    else:
        new_pop.append(pop[Current_elite])

# Find a random subset of 3 individuals and pick the fittest
def Tournament_Selection(parent):
    mating_subset = []
    mating_subset.append(parent)
    # While the parent is in the subset, clear it, get new subset
    while parent in mating_subset:
        mating_subset.clear()
        for i in range(3):
            mating_subset.append(random.randrange(0, len(pop), 1))
    
    temp = 0
    for i in range(len(mating_subset)):
        current_selction = sum(pop[mating_subset[i]])
        if current_selction > temp:
            temp = current_selction
            partner = mating_subset[i]
    return partner

def Mutation(offspring_index):
    if random.random() <=0.10:
        mutate_index = random.randint(0, 19)
        if new_pop[offspring_index][mutate_index] == 1:
            new_pop[offspring_index][mutate_index] = 0
        else: 
            new_pop[offspring_index][mutate_index] = 1

# Rows
M = 20
# Columns
N = 10
# Generate a population of N with Genome M
pop = []
for i in range (N):
    pop.append([random.randint(0,1) for j in range(M)])
# New population
new_pop = []
Generation = 0
Flag = False

while not (Flag or Generation == 150):
    tracker = 0
    Flag = Fitness()
    # If Fitness() -> has found all 1's -> Break
    if Flag == True:
        break
    print('Generation: ' + str(Generation))
    while len(new_pop) < len(pop):
        for i in range(len(pop)):
            if random.random() <= 0.50 and len(pop) > len(new_pop):
                tracker += 2
                Offsring1 = tracker - 2
                Offsring2 = tracker - 1
                mate = Tournament_Selection(i)
                new_pop.append(pop[i][0:10] + pop[mate][10:20])
                new_pop.append(pop[mate][0:10] + pop[i][10:20])
                Mutation(Offsring1)
                Mutation(Offsring2)
    pop.clear()
    for i in range(len(new_pop)):
        print(str(i) + ': ' + str(new_pop[i]) )
        pop.append(new_pop[i])
    new_pop.clear()
    Generation += 1

print('Done!')