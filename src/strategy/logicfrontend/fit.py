#!/usr/bin/env python
# -*- encoding:utf-8 -*-

import sys
import numpy as np
# import scipy as sp
# from scipy.stats import norm
from sklearn.pipeline import Pipeline
from sklearn.linear_model import LinearRegression
from sklearn.preprocessing import PolynomialFeatures
from sklearn import linear_model
from redis import Redis

import warnings
warnings.filterwarnings('ignore')

class KSignal():
    """信号管理"""
    def __init__(self, degree):
        self.degree = int(degree)
        self.sList = []

    def push(self, singal):
        self.sList.append(singal)

    def fit(self):
        x = np.arange(len(self.sList))
        y = np.array(self.sList)
        clf = Pipeline([('poly', PolynomialFeatures(degree=self.degree)),
            ('linear', LinearRegression(fit_intercept=False))])
        clf.fit(x[:, np.newaxis], y)
        yfited = clf.predict(x[:, np.newaxis])
        std = np.std(y - yfited)

        f, s = self.spectrum(y - yfited)
        sMax = np.max(s)
        s = s.tolist()
        sIdx = s.index(sMax)
        f = f.tolist()
        fsMax = f[sIdx] if f else 0
        return std, yfited.tolist(), fsMax

    def spectrum(self, nList):
        dt = 0.5
        n = len(nList)
        fx = np.fft.fft(nList)
        fx = fx[0:int(round(n/2)) - 1]
        s = abs(fx) ** 2
        f = np.arange(0, (round(n/2)-1)/(n * dt), 1/(n * dt))
        s = s / sum(s) * (n * dt)
        return f, s

config = {
    'hc1610': {'degree': 3, 'stdN': 0.5}
}

data = sys.argv[1]

dataArr = data.split('|')
krange = dataArr.pop()
iid = dataArr.pop()
# degree = config[iid]['degree']
# stdN = config[iid]['stdN']
degree = 3
stdN = 0.5
ksignal = KSignal(degree)
for s in dataArr:
    ksignal.push(float(s))
std, fitList, fsMax = ksignal.fit()

fit1 = fitList.pop()
fit2 = fitList.pop()
offset = std * stdN
rsp = str(fit1) + '|' + str(fit2) + '|' + str(offset) + '|' + str(fsMax)
# print rsp
rds = Redis(host='127.0.0.1', port=6379, db=1)
rds.set('FIT_RSP', rsp)



