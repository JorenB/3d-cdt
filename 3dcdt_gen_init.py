#!/usr/bin/env python
# coding: utf-8
# parameters: g T outfile


import sys

import gc

import numpy as np
import pylab as pl
import math as m


import time
import random
import itertools
import re

import fileinput


# I.      X*t - N1TL + N2TL - N22 = 0
# 
# II      N2SL + N2TL  = 2(N31+N22)
# 
# III.    N31 = 4/3 N1SL = 2N2SL
# 
# IV.     N1SL = 3/2 N2SL
# 
# V.      N0 = X*t + 1/2 N2SL
# 
# I = 2(N31+N22) + N22 + N1TL - X*t = 0
# 
# 2N3 + N22 + N1TL - X*t = 0
# 
# N31 = -3/2 N22 - 1/2 N1TL + X*t = 0

# In[3]:


def prepare_vertex_array_sphere(T):

    start = 0
    V = [] 
    for t in range(T):
        v = np.arange(start,start+5,1)
        start = max(v)+1
        V.append(v)

    vertex = np.zeros(T*5)

    for t in range(T):
        for item in V[t]:
            vertex[item] = t

    return vertex


# In[4]:


def prepare_vertex_array(g,T):
    V = []
    start = 0
    v = GetInnerVertices(g,T,start)
    V.append(v)

    for s in range(1,g,1):
        start = max(v.flatten())+1
        v = GetInnerVertices(g,T,start)
        V.append(v)
    
    Missing = GetMissingVertices(g,T)
    missing_max = max(np.concatenate(Missing).flatten())+1
    Corner = GetCornerVertices(g,T,missing_max)
    
    vertex = np.zeros(1+max(Corner.flatten()))
   
    for structure in V:
         for t in range(T):
            for item in structure[t]:
                vertex[item] = t
    if g > 1:
        for structure in Missing:
            for t in range(T):
                for item in structure[t]:
                    vertex[item] = t

        for structure in Corner:
            for t in range(T):
                for item in structure[t]:
                    vertex[item] = t
    if g == 1:
        
        for t in range(T):
            for item in Missing[t]:
                vertex[item] = t

            for item2 in Corner[t]:
                vertex[item2] = t
        
    return np.asarray(vertex,dtype = 'int')


def FindTrianglePairs(list_):
    pairs = []

    for i in range(len(list_)):
        p0 = list_[i][0]
        p1 = list_[i][1]
        p2 = list_[i][2]
        p3 = list_[i][3]   

        lista0 = np.argwhere(list_.T == p0).T[1]
        lista1 = np.argwhere(list_.T == p1).T[1]
        lista2 = np.argwhere(list_.T == p2).T[1]

        ov01 = list((set(lista0).intersection(lista1)))
        ov12 = list((set(lista1).intersection(lista2)))

        triangle = list((set(ov01).intersection(ov12)))


        pairs.append(triangle)
    return np.asarray(pairs)


def getTrianglelList(list3):
    list2 = []
    for i in range(len(list3)):
        simp = list3[i]
        
        t1 = list(np.sort(np.asarray([simp[0],simp[1],simp[2]])))
        t2 = list(np.sort(np.asarray([simp[0],simp[1],simp[3]])))
        t3 = list(np.sort(np.asarray([simp[0],simp[2],simp[3]])))
        t4 = list(np.sort(np.asarray([simp[1],simp[2],simp[3]])))
        
        if t1 not in list2:
            list2.append(t1)
            
        if t2 not in list2:
            list2.append(t2)

        if t3 not in list2:
            list2.append(t3)

        if t4 not in list2:
            list2.append(t4)
    
    return np.asarray(list2)

def getLinklList(list2):
    list1 = []
    
    for i in range(len(list2)):
        
        tria = list2[i]
        el = [tria[0],tria[1]]
        
        if el not in list1:
            list1.append(el)
        el = [tria[0],tria[2]]
        
        if el not in list1:
            list1.append(el)
        el = [tria[1],tria[2]]
        
        if el not in list1:
            list1.append(el)
    
    return np.asarray(list1)


def PrepDat(vertexlist,simplexlist,list_):
    dat = []
    dat.append(len(vertexlist))
    for item in vertexlist:
        dat.append(item)


    dat.append(len(vertexlist))
    dat.append(len(simplexlist))

    for i in range(len(simplexlist)):
        for item in list_[i]:
            dat.append(item)
        for item in simplexlist[i]:
            dat.append(item)

    dat.append(len(simplexlist))
    return np.asarray(dat)


# ALL the blocks are proper

def GetBlocks_up_left(a,b,c,d,e,f):
    s1 = np.asarray([a,b,c,d])
    s2 = np.asarray([b,c,d,e])
    s3 = np.asarray([c,d,e,f])
    return s1,s2,s3

def GetBlocks_up_right(a,b,c,d,e,f):
    s1 = np.asarray([a,b,c,d])
    s2 = np.asarray([b,c,d,f])
    s3 = np.asarray([b,d,e,f])
    return s1,s2,s3

def GetBlocks_down_left(a,b,c,d,e,f):
    s1 = np.asarray([a,b,c,e])
    s2 = np.asarray([a,c,e,f])
    s3 = np.asarray([a,d,e,f])
    return s1,s2,s3

def GetBlocks_down_right(a,b,c,d,e,f):
    s1 = np.asarray([a,b,c,f])
    s2 = np.asarray([a,b,e,f])
    s3 = np.asarray([a,d,e,f])
    return s1,s2,s3


# In[6]:


def GetTR(list3):
    TR = []
    bad_tetra = []
    for i in range(len(list3)):
        triangles = []

        tetra = list3[i]

        triangles.append(np.asarray([tetra[0],tetra[1],tetra[2]]))
        triangles.append(np.asarray([tetra[0],tetra[1],tetra[2]]))
        triangles.append(np.asarray([tetra[0],tetra[2],tetra[3]]))
        triangles.append(np.asarray([tetra[1],tetra[2],tetra[3]]))
        triangles = np.asarray(triangles)


        for tri in triangles:
            p1 = np.argwhere(list3.T==tri[0]).T[1]
            p2 = np.argwhere(list3.T==tri[1]).T[1]
            p3 = np.argwhere(list3.T==tri[2]).T[1]


            int12 = list((set(p1).intersection(p2)))
            int13 = list((set(p1).intersection(p3)))

            TR.append(len(list((set(int12).intersection(int13)))))
            
            if(len(list((set(int12).intersection(int13)))) != 2):
                bad_tetra.append(np.asarray([i,tri,len(list((set(int12).intersection(int13))))]))
                

    not2 = [i for i in range(len(TR)) if TR[i] != 2 ]
    
    return np.asarray(not2),np.asarray(bad_tetra)


# In[7]:


def GetInnerVertices(g,T,start):

    V = []
    prev_max = start
    for t in range(T):
        v_cur = np.arange(prev_max,prev_max + 12,1)
        prev_max = max(v_cur)+1

        V.append(v_cur)
    
    V.append(np.arange(start,start + 12,1))
    V = np.asarray(V)

    return V


def GenCenter(g,T):

    start = 0
    Simps = []
    v = GetInnerVertices(g,T,start)
    Simps.append(GenInnerStructure(v))
    
    for s in range(1,g,1):
        start = max(v.flatten())+1
        v = GetInnerVertices(g,T,start)
        Simps.append(GenInnerStructure(v))
        
    Simps = np.asarray(Simps) 
    
    Simps = np.reshape(Simps,(((np.shape(Simps)[0]*np.shape(Simps)[1],4))))
        
    
    return Simps

def GenInnerStructure(v):
    
    Simps = []
    for t in range(T):
        vv = v[t]
        vv_2 = v[t+1]

        a0 = vv[0]
        a1 = vv[1]
        a2 = vv[2]
        a3 = vv[3]
        a4 = vv[4]
        a5 = vv[5]
        a6 = vv[6]
        a7 = vv[7]
        a8 = vv[8]
        a9 = vv[9]
        a10 = vv[10]
        a11 = vv[11]

        a0_2 = vv_2[0]
        a1_2 = vv_2[1]
        a2_2 = vv_2[2]
        a3_2 = vv_2[3]
        a4_2 = vv_2[4]
        a5_2 = vv_2[5]
        a6_2 = vv_2[6]
        a7_2 = vv_2[7]
        a8_2 = vv_2[8]
        a9_2 = vv_2[9]
        a10_2 = vv_2[10]
        a11_2 = vv_2[11]


        s = GetBlocks_up_right(a4,a0,a1,a4_2,a0_2,a1_2)
        Simps.append(s[0])
        Simps.append(s[1])
        Simps.append(s[2])
        s = GetBlocks_up_right(a5,a1,a2,a5_2,a1_2,a2_2)
        Simps.append(s[0])
        Simps.append(s[1])
        Simps.append(s[2])
        s = GetBlocks_up_right(a6,a2,a3,a6_2,a2_2,a3_2)
        Simps.append(s[0])
        Simps.append(s[1])
        Simps.append(s[2])
        s = GetBlocks_up_right(a7,a3,a0,a7_2,a3_2,a0_2)
        Simps.append(s[0])
        Simps.append(s[1])
        Simps.append(s[2])
        s = GetBlocks_up_left(a8,a1,a0,a8_2,a1_2,a0_2)
        Simps.append(s[0])
        Simps.append(s[1])
        Simps.append(s[2])
        s = GetBlocks_up_left(a9,a2,a1,a9_2,a2_2,a1_2)
        Simps.append(s[0])
        Simps.append(s[1])
        Simps.append(s[2])
        s = GetBlocks_up_left(a10,a3,a2,a10_2,a3_2,a2_2)
        Simps.append(s[0])
        Simps.append(s[1])
        Simps.append(s[2])
        s = GetBlocks_up_left(a11,a0,a3,a11_2,a0_2,a3_2)
        Simps.append(s[0])
        Simps.append(s[1])
        Simps.append(s[2])
        s = GetBlocks_down_right(a0,a4,a7,a0_2,a4_2,a7_2)
        Simps.append(s[0])
        Simps.append(s[1])
        Simps.append(s[2])
        s = GetBlocks_down_right(a1,a5,a4,a1_2,a5_2,a4_2)
        Simps.append(s[0])
        Simps.append(s[1])
        Simps.append(s[2])
        s = GetBlocks_down_right(a2,a6,a5,a2_2,a6_2,a5_2)
        Simps.append(s[0])
        Simps.append(s[1])
        Simps.append(s[2])
        s = GetBlocks_down_right(a3,a7,a6,a3_2,a7_2,a6_2)
        Simps.append(s[0])
        Simps.append(s[1])
        Simps.append(s[2])
        s = GetBlocks_down_right(a0,a11,a8,a0_2,a11_2,a8_2)
        Simps.append(s[0])
        Simps.append(s[1])
        Simps.append(s[2])
        s = GetBlocks_down_right(a1,a8,a9,a1_2,a8_2,a9_2)
        Simps.append(s[0])
        Simps.append(s[1])
        Simps.append(s[2])
        s = GetBlocks_down_right(a2,a9,a10,a2_2,a9_2,a10_2)
        Simps.append(s[0])
        Simps.append(s[1])
        Simps.append(s[2])
        s = GetBlocks_down_right(a3,a10,a11,a3_2,a10_2,a11_2)
        Simps.append(s[0])
        Simps.append(s[1])
        Simps.append(s[2])
        
        
    Simps = np.asarray(Simps)
    
    return Simps


def GenInnerOuters(g,T):
    start = 0
    struct = []
    v = GetInnerVertices(g,T,start)
    struct.append(v.T[4:].T)

    for s in range(1,g,1):
        start = max(v.flatten())+1
        v = GetInnerVertices(g,T,start)
        struct.append(v.T[4:].T)
    return np.asarray(struct)


def GetVsingle(g,T):
    cur_max = 12*g*T

    Vsingle = []
    for t in range(T):    
        v = []
        for i in range(8):        
            if i < 4:
                v.append(cur_max)
                cur_max+=1
            if i == 4:
                v.append(v[0])
            if i == 5:
                v.append(v[1])
            if i == 6:
                v.append(v[2])
            if i == 7:
                v.append(v[3])

        Vsingle.append(v)
    Vsingle.append(Vsingle[0])
    return Vsingle

def GetV0(cur):
    cur_max = cur
    V0 = []
    for t in range(T):    
        v = []
        for i in range(8):        
            if i < 4:
                v.append(cur_max)
                cur_max+=1

            if i == 4:
                v.append(v[0])
            if i == 5:
                v.append(cur_max)
                cur_max+=1
            if i == 6:
                v.append(v[2])
            if i == 7:
                v.append(v[3])

        V0.append(np.asarray(v))
    V0.append(V0[0])
 
    return np.asarray(V0)


def GetVmid(Vpre):

    Vprev = Vpre
    cur_max = max(np.concatenate(Vpre))+1

    Vmidg = []
    for t in range(T):    
        v = []
        for i in range(8):        
            if i < 3:
                v.append(cur_max)
                cur_max+=1
            if i == 3:
                v.append(Vprev[t][1])
            if i == 4:
                v.append(v[0])
            if i == 5:
                v.append(cur_max)
                cur_max+=1
            if i == 6:
                v.append(v[2])
            if i == 7:
                v.append(Vprev[t][5])

        Vmidg.append(v)
    Vmidg.append(Vmidg[0])

    return np.asarray(Vmidg)

def GetVlast(Vpre):

    Vprev = Vpre
    cur_max = max(np.concatenate(Vprev))+1

    Vlast = []
    for t in range(T):    
        v = []
        for i in range(8):        
            if i < 3:
                v.append(cur_max)
                cur_max+=1
            if i == 3:
                v.append(Vprev[t][1])
            if i == 4:
                v.append(v[0])
            if i == 5:
                v.append(v[1])
            if i == 6:
                v.append(v[2])
            if i == 7:
                v.append(Vprev[t][5])

        Vlast.append(v)
    Vlast.append(Vlast[0])

    return np.asarray(Vlast)

def GetCornerVertices(g,T,cur_max):
    VV = []
    v_cur = cur_max

    if g == 1:
        
        for t in range(T):
            v = []
            v.append(v_cur)
            v.append(v_cur+1)
            v.append(v_cur+2)
            v.append(v_cur+3)
            
            v_cur += 4 
            
            VV.append(v)

        VV.append(VV[0])
        return np.asarray(VV)
        
    for s in range(g):
        V = []
        for t in range(T):
            v = []
            if s == 0:
                v.append(v_cur)
                v_cur += 1
                v.append(v_cur)
                v_cur += 1
                v.append(v_cur)
                v_cur += 1
                v.append(v_cur)
                v_cur += 1

                V.append(v)

            else:
                v.append(VV[s-1][t][2])
                v.append(VV[s-1][t][3])
                v.append(v_cur)
                v_cur += 1
                v.append(v_cur)
                v_cur += 1

                V.append(v)
                
        V.append(V[0])
        VV.append(np.asarray(V))
 
    return np.asarray(VV)


def GetMissingVertices(g,T):
    V = []
    if g == 1:
        V=GetVsingle(g,T)
        
    if g == 2:
        V0 = GetV0(12*g*T)
        Vlast = GetVlast(V0)
        
        V.append(V0)
        V.append(Vlast)
    
    if g > 2:
        
        V0 = GetV0(12*g*T)
        V.append(V0)
    
        for s in range(1,g-1,1):
            if s == 1:
                Vmid = GetVmid(V0)
                V.append(Vmid)
            else:
                Vmid = GetVmid(Vmid)
                V.append(Vmid)
        
        Vlast = GetVlast(Vmid)
        V.append(Vlast)
        
    return np.asarray(V)


def GenMissing(g,T):

    Inner = GenInnerOuters(g,T)
    Missing = GetMissingVertices(g,T)
    missing_max = max(np.concatenate(Missing).flatten())+1
    Corner = GetCornerVertices(g,T,missing_max)

   
    S = []
    for s in range(g):
        
        if g > 1:
            inners = Inner[s]
            missings = Missing[s]
            corners = Corner[s]
            
        if g == 1:
            inners = Inner[0]
            missings = Missing
            corners = Corner
            
        for t in range(T):
            inner_v = inners[t]    
            missing_v = missings[t]
            corner_v = corners[t]

            inner_v2 = inners[t+1]    
            missing_v2 = missings[t+1]
            corner_v2 = corners[t+1]

            a0,a1,a2,a3,a4,a5,a6,a7 = inner_v[0],inner_v[1],inner_v[2],inner_v[3],inner_v[4],inner_v[5],inner_v[6],inner_v[7]
            a0_2,a1_2,a2_2,a3_2,a4_2,a5_2,a6_2,a7_2 = inner_v2[0],inner_v2[1],inner_v2[2],inner_v2[3],inner_v2[4],inner_v2[5],inner_v2[6],inner_v2[7]

            m0,m1,m2,m3,m4,m5,m6,m7 = missing_v[0],missing_v[1],missing_v[2],missing_v[3],missing_v[4],missing_v[5],missing_v[6],missing_v[7]
            m0_2,m1_2,m2_2,m3_2,m4_2,m5_2,m6_2,m7_2 = missing_v2[0],missing_v2[1],missing_v2[2],missing_v2[3],missing_v2[4],missing_v2[5],missing_v2[6],missing_v2[7]

            c0,c1,c2,c3 = corner_v[0],corner_v[1],corner_v[2],corner_v[3]
            c0_2,c1_2,c2_2,c3_2 = corner_v2[0],corner_v2[1],corner_v2[2],corner_v2[3]


            s = GetBlocks_up_left(c0,m3,a3,c0_2,m3_2,a3_2)
            S.append(s[0])
            S.append(s[1])
            S.append(s[2])
            s = GetBlocks_up_left(c0,a3,a0,c0_2,a3_2,a0_2)
            S.append(s[0])
            S.append(s[1])
            S.append(s[2])
            s = GetBlocks_up_left(c0,a0,m0,c0_2,a0_2,m0_2)
            S.append(s[0])
            S.append(s[1])
            S.append(s[2])
            s = GetBlocks_up_right(c2,m0,a0,c2_2,m0_2,a0_2)
            S.append(s[0])
            S.append(s[1])
            S.append(s[2])
            s = GetBlocks_up_left(c2,a0,a1,c2_2,a0_2,a1_2)
            S.append(s[0])
            S.append(s[1])
            S.append(s[2])
            s = GetBlocks_up_right(c2,a1,m1,c2_2,a1_2,m1_2)
            S.append(s[0])
            S.append(s[1])
            S.append(s[2])
            s = GetBlocks_up_left(c3,m1,a1,c3_2,m1_2,a1_2)
            S.append(s[0])
            S.append(s[1])
            S.append(s[2])
            s = GetBlocks_up_left(c3,a1,a2,c3_2,a1_2,a2_2)
            S.append(s[0])
            S.append(s[1])
            S.append(s[2])
            s = GetBlocks_up_left(c3,a2,m2,c3_2,a2_2,m2_2)
            S.append(s[0])
            S.append(s[1])
            S.append(s[2])
            s = GetBlocks_up_right(c1,m2,a2,c1_2,m2_2,a2_2)
            S.append(s[0])
            S.append(s[1])
            S.append(s[2])
            s = GetBlocks_up_left(c1,a2,a3,c1_2,a2_2,a3_2)
            S.append(s[0])
            S.append(s[1])
            S.append(s[2])
            s = GetBlocks_up_right(c1,a3,m3,c1_2,a3_2,m3_2)
            S.append(s[0])
            S.append(s[1])
            S.append(s[2])
            s = GetBlocks_up_right(c1,a6,m6,c1_2,a6_2,m6_2)
            S.append(s[0])
            S.append(s[1])
            S.append(s[2])
            s = GetBlocks_up_left(c1,a7,a6,c1_2,a7_2,a6_2)
            S.append(s[0])
            S.append(s[1])
            S.append(s[2])
            s = GetBlocks_up_right(c1,m7,a7,c1_2,m7_2,a7_2)
            S.append(s[0])
            S.append(s[1])
            S.append(s[2])
            s = GetBlocks_up_left(c0,a7,m7,c0_2,a7_2,m7_2)
            S.append(s[0])
            S.append(s[1])
            S.append(s[2])
            s = GetBlocks_up_left(c0,a4,a7,c0_2,a4_2,a7_2)
            S.append(s[0])
            S.append(s[1])
            S.append(s[2])
            s = GetBlocks_up_left(c0,m4,a4,c0_2,m4_2,a4_2)
            S.append(s[0])
            S.append(s[1])
            S.append(s[2])
            s = GetBlocks_up_right(c2,a4,m4,c2_2,a4_2,m4_2)
            S.append(s[0])
            S.append(s[1])
            S.append(s[2])
            s = GetBlocks_up_left(c2,a5,a4,c2_2,a5_2,a4_2)
            S.append(s[0])
            S.append(s[1])
            S.append(s[2])
            s = GetBlocks_up_right(c2,m5,a5,c2_2,m5_2,a5_2)
            S.append(s[0])
            S.append(s[1])
            S.append(s[2])
            s = GetBlocks_up_left(c3,m6,a6,c3_2,m6_2,a6_2)
            S.append(s[0])
            S.append(s[1])
            S.append(s[2])
            s = GetBlocks_up_left(c3,a6,a5,c3_2,a6_2,a5_2)
            S.append(s[0])
            S.append(s[1])
            S.append(s[2])
            s = GetBlocks_up_left(c3,a5,m5,c3_2,a5_2,m5_2)
            S.append(s[0])
            S.append(s[1])
            S.append(s[2])

    return np.asarray(S)

def GenSphere(T):
    start = 0
    V = [] 
    for t in range(T):
        v = np.arange(start,start+5,1)
        start = max(v)+1
        V.append(v)

    V.append(np.arange(0,5,1))

    V = np.asarray(V)
    Simps = []

    for t in range(T):
        vt = V[t]
        vtp1 = V[t+1]

        s0 = vt[0]
        s1 = vt[1]
        s2 = vt[2]
        s3 = vt[3]
        s4 = vt[4]      

        s0_2 = vtp1[0]
        s1_2 = vtp1[1]
        s2_2 = vtp1[2]
        s3_2 = vtp1[3]
        s4_2 = vtp1[4]
        
        s = GetBlocks_up_right(s0,s1,s2,s0_2,s1_2,s2_2) # center
        Simps.append(s[0])
        Simps.append(s[1])
        Simps.append(s[2])
        s = GetBlocks_up_right(s0,s3,s1,s0_2,s3_2,s1_2)
        Simps.append(s[0])
        Simps.append(s[1])
        Simps.append(s[2])
        s = GetBlocks_up_right(s0,s2,s3,s0_2,s2_2,s3_2)
        Simps.append(s[0])
        Simps.append(s[1])
        Simps.append(s[2])
       
        
        s = GetBlocks_up_left(s4,s2,s1,s4_2,s2_2,s1_2) # closure
        Simps.append(s[0])
        Simps.append(s[1])
        Simps.append(s[2])
        s = GetBlocks_up_left(s4,s1,s3,s4_2,s1_2,s3_2)
        Simps.append(s[0])
        Simps.append(s[1])
        Simps.append(s[2])
        s = GetBlocks_up_left(s4,s3,s2,s4_2,s3_2,s2_2)
        Simps.append(s[0])
        Simps.append(s[1])
        Simps.append(s[2])
        
    return np.asarray(Simps)

def FindPairs(list_):
    S = []
    for i in range(len(list_)):
        TRI = []
        simp = list_[i]

        p0 = simp[0]
        p1 = simp[1]
        p2 = simp[2]
        p3 = simp[3]   

        TRI.append([p0,p1,p2])
        TRI.append([p0,p1,p3])
        TRI.append([p0,p2,p3])
        TRI.append([p1,p2,p3])

        A = []

        for tri in TRI:
            tp0 = tri[0]
            tp1 = tri[1]
            tp2 = tri[2]

            lista0 = np.argwhere(list3.T == tp0).T[1]
            lista1 = np.argwhere(list3.T == tp1).T[1]
            lista2 = np.argwhere(list3.T == tp2).T[1]

            ov01 = list((set(lista0).intersection(lista1)))
            ov12 = list((set(lista1).intersection(lista2)))

            triangle = list((set(ov01).intersection(ov12)))

            A.append(triangle)
        p = np.unique(A)
        f = []
        for item in p:
            if item != i:
                f.append(item)

        f = np.asarray(f)

        S.append(f)

    S = np.asarray(S)

    return S



g = int(sys.argv[1])
T = int(sys.argv[2])

if g == 0:
    list3 = GenSphere(T)

else:
    S_center = GenCenter(g,T)
    S_missing = GenMissing(g,T)
    list3 = np.vstack((S_center,S_missing))

    
list2 = getTrianglelList(list3)
list1 = getLinklList(list2)

num0 = max(list3.flatten())+1

X = len(list3)-len(list2)+len(list1)-num0

print("\n N3,N2,N1,N0,X: ",len(list3),len(list2),len(list1),num0,X)

Simplex_list = FindPairs(list3)


if g > 0:
    vertex = prepare_vertex_array(g,T)
else:
    vertex = prepare_vertex_array_sphere(T)


dat = PrepDat(vertex,Simplex_list,list3)


outfile = sys.argv[3]


with open(outfile, 'w') as file_handler:
    file_handler.write("0\n")
    for item in dat:
        file_handler.write("{}\n".format(int(item)))


