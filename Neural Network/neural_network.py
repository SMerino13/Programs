import csv
import math
from random import seed
from random import random

class Hidden_Neuron: 
    def __init__(self, x1, x2, x3, x4, w1, w2, w3, w4, thta ):
        self.x = (x1 * w1) + (x2 * w2) + (x3 * w3) + (x4 *w4) - thta
        self.y = 1 / (1 + math.exp(-self.x))
        
class Output_Neuron:
    def __init__(self, y1, y2, y3, w1, w2, w3, thta):
        self.Ox = (y1 * w1) + (y2 * w2) + (y3 * w3) - thta 
        self.y = 1 / (1 + math.exp(-self.Ox))

class Backpropagation:
    def Output_backprop(self, yd, y, h0, h1, h2, O_w1, O_w2, O_w3, O_thta):
        self.delta = y * (1 - y) * (yd - y)
        self.New_w1 = O_w1 + alpha * h0 * self.delta
        self.New_w2 = O_w2 + alpha * h1 * self.delta
        self.New_w3 = O_w2 + alpha * h2 * self.delta
        self.New_thta = O_thta + alpha * (-1) * self.delta
    def Hidden_backprop(self, y, Out_w1, Out_w2, Out_w3, w1, w2, w3, w4, thta, Out_delta1, Out_delta2, Out_delta3, x1, x2, x3, x4):
        self.delta = y * (1 - y) * (Out_w1 * Out_delta1 + Out_w2 * Out_delta2 + Out_w3 * Out_delta3)
        self.HNew_w1 = w1 + alpha * x1 * self.delta
        self.HNew_w2 = w2 + alpha * x2 * self.delta
        self.HNew_w3 = w3 + alpha * x3 * self.delta
        self.HNew_w4 = w4 + alpha * x4 * self.delta
        self.HNew_thta = thta + alpha * (-1) * self.delta
    def da_Hidden_Weights(self, Num):
        w[Num][0] = self.HNew_w1
        w[Num][1] = self.HNew_w2
        w[Num][2] = self.HNew_w3
        w[Num][3] = self.HNew_w4
        theta[Num][0] = self.HNew_thta
    def da_Output_Weights(self, Num):
        w[Num][0] = self.New_w1
        w[Num][1] = self.New_w2
        w[Num][2] = self.New_w3
        theta[Num][0] = self.New_thta

FILENAME = 'iris.csv'
with open(FILENAME, newline='') as csvfile:
    vector = []

    for row in csv.reader(csvfile, delimiter=','):
        vector.append(row)

seed(1)
w = []
theta = []
for i in range(6):
    w.append([random() for j in range(4)])
    theta.append([random()])
alpha = 0.1

for epoch in range(100):
    i = 0
    daSum = 0
    for sample in vector:
        inputs = vector[i][:4]
        yd3 = float(vector[i][4])
        yd4 = float(vector[i][5])
        yd5 = float(vector[i][6])

        # Calculate charge for Hidden Neurons
        h0 = Hidden_Neuron(float(inputs[0]), float(inputs[1]), float(inputs[2]), float(inputs[3]), w[0][0], w[0][1], w[0][2], w[0][3], theta[0][0])
        h1 = Hidden_Neuron(float(inputs[0]), float(inputs[1]), float(inputs[2]), float(inputs[3]), w[1][0], w[1][1], w[1][2], w[1][3], theta[1][0])
        h2 = Hidden_Neuron(float(inputs[0]), float(inputs[1]), float(inputs[2]), float(inputs[3]), w[2][0], w[2][1], w[2][2], w[2][3], theta[2][0])
        # Calculate charge for Output Neurons
        O3 = Output_Neuron(h0.y, h1.y, h2.y, w[3][0], w[3][1], w[3][2], theta[3][0])
        O4 = Output_Neuron(h0.y, h1.y, h2.y, w[4][0], w[4][1], w[4][2], theta[4][0])
        O5 = Output_Neuron(h0.y, h1.y, h2.y, w[5][0], w[5][1], w[5][2], theta[5][0])

        B3 = Backpropagation()
        B4 = Backpropagation()
        B5 = Backpropagation()
        B3.Output_backprop(yd3, O3.y, h0.y, h1.y, h2.y, w[3][0], w[3][1], w[3][2], theta[3][0])
        B4.Output_backprop(yd4, O4.y, h0.y, h1.y, h2.y, w[4][0], w[4][1], w[4][2], theta[4][0])
        B5.Output_backprop(yd5, O5.y, h0.y, h1.y, h2.y, w[5][0], w[5][1], w[5][2], theta[5][0])

        H0 = Backpropagation()
        H1 = Backpropagation()
        H2 = Backpropagation()
        H0.Hidden_backprop(h0.y, w[3][0], w[4][0], w[5][0], w[0][0], w[0][1], w[0][2], w[0][3], theta[0][0], B3.delta, B4.delta, B5.delta, 
            float(inputs[0]), float(inputs[1]), float(inputs[2]), float(inputs[3]))

        H1.Hidden_backprop(h1.y, w[3][1], w[4][1], w[5][1], w[1][0], w[1][1], w[1][2], w[1][3], theta[1][0], B3.delta, B4.delta, B5.delta, 
            float(inputs[0]), float(inputs[1]), float(inputs[2]), float(inputs[3]))

        H2.Hidden_backprop(h2.y, w[3][2], w[4][2], w[5][2], w[2][0], w[2][1], w[2][2], w[2][3], theta[2][0], B3.delta, B4.delta, B5.delta, 
            float(inputs[0]), float(inputs[1]), float(inputs[2]), float(inputs[3]))

        H0.da_Hidden_Weights(0)
        H1.da_Hidden_Weights(1)
        H2.da_Hidden_Weights(2)
        B3.da_Output_Weights(3)
        B4.da_Output_Weights(4)
        B5.da_Output_Weights(5)

        daSum += abs(yd3 - O3.y) + abs(yd4 - O4.y) + abs(yd5 - O5.y) 
        i += 1
        print('Epoch ' + str(epoch) + ' Iteration ' + str(i) + ': Prediction is [' + str(round(O3.y, 2)) + ' ' + str(round(O4.y, 2)) + ' ' + str(round(O5.y, 2)) + ']')
    
    print('Epoch ' + str(epoch) + ' Results: MAD = ' + str(daSum / len(vector)) )