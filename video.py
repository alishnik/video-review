# %load new_model.py

import numpy as np
import pandas as pd

import random
import math
import matplotlib
import matplotlib.pyplot as plt
import subprocess
import sys, os, argparse
import pickle as pkl
import copy
import numbers
import argparse

from collections import Counter, defaultdict
from itertools import product
from fractions import gcd
from ast import literal_eval

import logging
import time

class Printer:
    def __init__(self, verbose, owner):
        """
        Класс, ответственный за распечатку сообщений. 
        Печатает сообщение, когда его уровень печати превосходит уноверь печати
        объекта класса.
        """
        self.verbose = verbose
        self.owner = owner
    def __call__(self, *args, **kwargs):
        if self.owner.verbose >= self.verbose:
            print(*args, **kwargs)


class Checker:
    def _check_int(self, n, name):
        if not n == int(n):
            raise TypeError("Param \"{}\" must be an integer number".format(name))
        return True
    def _check_numeric(self, n, name):
        if not isinstance(n, numbers.Number):
            raise TypeError("Param \"{}\" must be a number".format(name))
        return True  
    def _check_positive(self, n, name):
        if n < 0:
            raise ValueError("Param \"{}\" must be positive".format(name))
        return True
    def _check_proba(self, n, name):
        self._check_numeric(n, name)
        if (n <= 1) & (n >= 0):
            return True
        raise ValueError("Param \"{}\" is not proba though it must be".format(name))
        
class BaseTracer(Checker):
    TR_INFO_0 = 4
    TR_INFO_1 = 5
    TR_INFO_2 = 6
    TR_INFO_3 = 7
    
    def __init__(self, Tin, Tres, D, PER):
        """
        :param Tin - период поступления
        :type Tin - int
        
        :param Tres - период резервирования
        :type Tin - int
        
        :param D - delay bound
        :type D - int
        
        :param PER - вероятность ошибки
        :type PER - float
        """
        
        self._check_numeric(Tin, 'Tin')
        self._check_numeric(Tres, 'Tres')
        self._check_numeric(D, 'D')

        self._check_int(Tin, 'Tin')
        self._check_int(D, 'D')
        self._check_int(Tres, 'Tres')
        
        self._check_positive(Tin, 'Tin')
        self._check_positive(Tres, 'Tres')
        self._check_positive(D, 'D')
        
        self._check_numeric(PER, 'PER')
        self._check_proba(PER, 'PER')
        
        self.statesprob = Counter()
        self.stateslost = Counter()
        self.statesarrived = Counter()
        
        self.Tin = Tin
        self.D = D
        self.Tres = Tres
        self.PER = PER
        self.n_iters = 0
        
        self.printers = {}
        for v in range(20):
            self.printers[v] = Printer(v, self)          
        
    def set_Tres(self, Tres):
        self._check_numeric(Tres, 'Tres')
        self._check_int(Tres, 'Tres')
        self._check_positive(Tres, 'Tres')     
        self.Tres = Tres
        
    def _reset(self, problist, lostlist, statesarrived):
        if problist is not None:
            self.statesprob = Counter()
            self.statesprob.update(problist)
        if lostlist is not None:
            self.stateslost = Counter()
            self.stateslost.update(lostlist)
        if statesarrived is not None:
            self.statesarrived = Counter()
            self.statesarrived.update(statesarrived)
    
    def __call__(self, W, trace, 
                 problist=None, lostlist=None, statesarrived=None, input_distr=None, 
                 min_tracked_proba=1e-8, 
                 verbose=-1, report_period=10000000):
        """
        Внимание:
        Изначально окно W начинается в момент времени 0, в котором также начинается некоторый зарезерированный 
        интервал. При повторных запусках окно W отсчитывается с резервирования, на котором 
        остановились предшествующие вычисления. Начальные offset-ыможно моделировать задав соответствующие
        problist и lostlist.
        
        Просчитываются все резервирования, начинающиеся в интервале[0, W). 
        Если резервироваание начинается в момент W и далее, то оно не рассматривается. Последующие 
        вычисления начинаются именно с него.
        
        При подсчете PLR число полученных пачек считается от 0-го момента, т.е. те, 
        что уже были в очереди, не в счет
        
        :param W - размер окна в тех же единицах, что и Tin, Tres
        :type W - int, float
        
        :param trace - трейс размеров пачек
        :type trace - numpy.array
        
        :param problist - вероятности начальных состояний
        :type problist - dict
        
        :param lostlist - потери начальных состояний
        :type lostlist - dict
        
        :param statesarrived - число поступивших пакетов
        :type statesarrived - dict
        
        :param input_distr - распределение пачек входного потока
        :type input_distr - numpy.array размера M + 1, где M - максимальный размер пачки

        :param verbose - уровень печати вспомогательных сообщений
        :type verbose - int
        
        :param report_period - период печати статистики
        :type reprot_period - int
        """
        
        self.verbose = verbose
        self.min_tracked_proba = min_tracked_proba
        self._reset(problist, lostlist, statesarrived)
        
        Nres = int(float(W) / self.Tres)
        self.printers[self.TR_INFO_0]("Anticipated number of iterations equals {}".format(Nres))
        
        if input_distr:
            if input_distr[0] != 0:
                raise ValueError("Input distribution must not include batches of size 0!")
        
        for nres in range(Nres):
            self.nres = nres
            self.n_iters += 1
            if (nres + 1) % report_period == 0:
                plr = self.get_plr()
                self.printers[self.TR_INFO_0](
                    'Number if iteration {}, state space size equals {}, curr plr = {}'.format(
                    nres + 1, len(self.statesprob), plr))

            self.next_statesprob = Counter()
            self.next_stateslost = Counter()
            self.next_statesarrived = Counter()
            
            for state, prob in self.statesprob.items():
                self.printers[self.TR_INFO_1]("\tTransitions from state {} with probability {}".format(
                    state, prob))

                losts = self.stateslost[state]
                arrived = self.statesarrived[state]
                h, m, n = state
                
                if m == 0:
                    self.printers[self.TR_INFO_2]("\t\tState {} is final state".format(state))
                    # Достигнуто финальное состояние, так как трейс закончился. Остаемся в том же состоянии.
                    next_state = state
                    tr_prob = 1
                    self._add_proba(state, next_state, tr_prob, losts, arrived)
                    continue 
                    
                if h < 0:
                    self.printers[self.TR_INFO_2]("\t\th = {} is negative".format(h))
                    next_h = h + self.Tres
                    next_state = (next_h, m, n)
                    tr_prob = 1
                    self._add_proba(state, next_state, tr_prob, losts, arrived)
                elif h <= self.D - self.Tres:
                    self.printers[self.TR_INFO_2]("\t\th = {} is in range [0, D - Tres]".format(state))
                    # Успешная передача
                    if m == 1:
                        # Размер пачки равен 1 - смена пачки
                        next_h = h + self.Tres - self.Tin
                        next_n = n + 1
                        if next_n < len(trace):
                            next_m = trace[n + 1]
                            assert next_m > 0, "Batch with index {} is not positive.".format(n + 1)
                            next_state = (next_h, next_m, next_n)
                            tr_prob = 1 - self.PER
                            self._add_proba(state, next_state, tr_prob, losts, arrived + next_m)
                        else:
                            # Переходим на самообслуживание
                            if input_distr:
                                # Трейс кончился - используем распределение
                                for next_m in range(1, len(input_distr)):
                                    next_state = (next_h, next_m, next_n)
                                    tr_prob = (1 - self.PER) * input_distr[next_m]
                                    self._add_proba(state, next_state, tr_prob, losts, arrived + next_m)
                            else:
                                # Трейс кончился - переходим в финальное состояние
                                next_m = 0
                                next_state = (next_h, next_m, next_n)
                                tr_prob = 1 - self.PER
                                self._add_proba(state, next_state, tr_prob, losts, arrived)
                    else:
                        # Размер пачки больше 1 - уменьшение пачки
                        next_state = (h + self.Tres, m - 1, n)
                        tr_prob = 1 - self.PER
                        self._add_proba(state, next_state, tr_prob, losts, arrived)
                        
                    # Неуспешная передача
                    next_state = (h + self.Tres, m, n)
                    tr_prob = self.PER
                    self._add_proba(state, next_state, tr_prob, losts, arrived)
                else:
                    self.printers[self.TR_INFO_2]("\t\th = {} is higher than D - Tres".format(state))
                    next_h = h + self.Tres - self.Tin
                    next_n = n + 1
                    if next_n < len(trace):
                        next_m = trace[n + 1]
                        next_state = (next_h, next_m, next_n)
                        tr_prob = 1
                        self._add_proba(state, next_state, tr_prob, losts + m - 1 + self.PER, arrived + next_m)
                    else:
                        # Переходим на самообслуживание
                        if input_distr:
                            # Трейс кончился - используем распределение
                            for next_m in range(1, len(input_distr)):
                                next_state = (next_h, next_m, next_n)
                                tr_prob = input_distr[next_m]
                                self._add_proba(state, next_state, tr_prob, losts + m - 1 + self.PER, arrived + next_m)
                        else:
                            # Трейс кончился - переходим в финальное состояние
                            next_m = 0
                            next_state = (next_h, next_m, next_n)
                            tr_prob = 1
                            self._add_proba(state, next_state, tr_prob, losts, arrived)
                            
            # Перенормировка
            s = sum(self.next_statesprob.values())
            assert np.allclose(s, 1), "Sum of probabilities equals {}".format(s)

            for state, prob in self.next_statesprob.items():
                self.next_statesprob[state] /= s
                self.next_stateslost[state] /= prob
                self.next_statesarrived[state] /= prob
                
            self.statesprob = self.next_statesprob
            self.stateslost = self.next_stateslost
            self.statesarrived = self.next_statesarrived

        # Время подвести итоги
        plr = self.get_plr()
        return plr
    
    def get_plr(self):
        PLR = 0
        for state, prob in self.statesprob.items():
            if self.statesarrived[state] == 0:
                raise ValueError("No packets arrived during transition to state {}." \
                                 "Please, provide longer window.".format(state))
            PLR += prob * self.stateslost[state] / self.statesarrived[state]
        return PLR
    
    def _add_proba(self, state, next_state, tr_prob, losts, arrived):
        prob = self.statesprob[state]
        p = prob * tr_prob
        if p >= self.min_tracked_proba:
            self.next_statesprob[next_state] += p
            self.next_stateslost[next_state] += p * losts
            self.next_statesarrived[next_state] += p * arrived
            self.printers[self.TR_INFO_3]("\t\t\tTransition to {} with proba {}".format(next_state, tr_prob))

    def print_probas(self):
        self._print_counter(self.statesprob)
    def print_losts(self):
        self._print_counter(self.stateslost)
    def print_arrived(self):
        self._print_counter(self.statesarrived)
        
    def _print_counter(self, d):
        for state, value in sorted(list(d.items()), key=lambda x: x[0]):
            print('\t', state, value)
   
#####    ####     ###      ####    #####
  #      #   #   #   #    #        #    
  #      ####    #####    #        #####
  #      #   #   #   #    #        #    
  #      #   #   #   #     ####    #####        
offset = 0
Tres = 45

parser = argparse.ArgumentParser()
parser.add_argument("--res_period", type=int, help="Reservation period")
parser.add_argument("--file", type=str, help="Name of video")
parser.add_argument("--MAX", default = 1000000, type=int, help="Max size of batch")
parser.add_argument("--arr_period", default = 480, type=int, help="arrival blocks period")
parser.add_argument("--delay_bound", default = 1200, type=int, help="Delay bound of packets")
parser.add_argument("--per", default = 0.3, type=float, help="the probability of packet failed transmission")
args = parser.parse_args()

Tres = args.res_period
name = args.file
M = args.MAX
Tin = args.arr_period
PER = args.per
D = args.delay_bound

trace = pd.read_csv('disk/inputs/6000/' + name + '.batch.dat', header=None).values.flatten()

######Slice batch#########
for i in range(len(trace)):
    if trace[i] > M:
        trace[i] = M
##########################

n_batches = len(trace)
problist = {(0, trace[0], 0): 1}
lostlist = {(0, trace[0], 0): 0}
tracer = BaseTracer(Tin, Tres, D, PER)
W = n_batches * Tin
plr = tracer(W, trace, problist, lostlist, verbose=4, report_period=500000)
    
with open("disk/outputs/6000/res/sliced/" + name + ".new_model.dat", "a") as myfile:
    myfile.write(str(Tres) + '\t' + str(plr) + '\n')
    print(plr)











































