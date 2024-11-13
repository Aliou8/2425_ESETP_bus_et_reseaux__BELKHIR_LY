from flask import Flask
from flask import render_template
from flask import request
from flask import jsonify
import serial
import json

tab_T = [] #tableau des températures
tab_P = [] #tableau des pressions


ser = serial.Serial("/dev/serial0",115200,timeout=1)

#temperature
@app.route('/api/temp/', methods=['GET', 'POST'])
def api_temp():
        
        
        resp = {
            "method":   request.method,
            "url" :  request.url,
            "args": request.args,
            "headers": dict(request.headers),
    }
        if request.method == 'POST' :
                ser.write(b'GET_T')             #envoie à la stm32 que l'on veut faire un GET_T
                tempo = ser.readline().decode() #récupérer la valeur envoyée par la stm32
                tab_T.append(tempo[:9])         #retire le \r\n et ajoute dans le tableau
                return jsonify(tab_T[-1])       #renvoie la valeur
        if request.method == 'GET' :
                return jsonify(tab_T)           #renvoie le tableau

@app.route('/api/temp/<int:index>', methods=['GET','DELETE'])
def api_temp_index(index=None):
        resp = {
            "method":   request.method,
            "url" :  request.url,
            "index" : index,
            "args": request.args,
            "headers": dict(request.headers),
        }
        if request.method == 'GET' :
                if (index < len(tab_T)):
                        return jsonify(tab_T[index])    #récupère la valeur du tableau à l'index
                else :
                        return jsonify("error : index out of range")
        if request.method == 'DELETE' :
                if (index < len(tab_T)):
                        return jsonify("la valeur "+tab_T.pop(index)+ " a été retirée") #récupère la valeur du tableau à l'index et supprime du tableau
                else :
                        return jsonify("error : index out of range")


#pression
@app.route('/api/pres/', methods=['GET', 'POST'])
def api_pres():
        
        
        resp = {
            "method":   request.method,
            "url" :  request.url,
            "args": request.args,
            "headers": dict(request.headers),
        }
        if request.method == 'POST' :
                ser.write(b'GET_P')             #envoie à la stm32 que l'on veut faire un GET_P
                tempo = ser.readline().decode() #récupérer la valeur envoyée par la stm32
                tab_P.append(tempo[:20])        #retire le \r\n et ajoute dans le tableau
                return jsonify(tab_P[-1])       #renvoie la valeur
        if request.method == 'GET' :
                return jsonify(tab_P)           #renvoie le tableau


@app.route('/api/pres/<int:index>', methods=['GET', 'DELETE'])
def api_pres_index(index=None):
        resp = {
            "method":   request.method,
            "url" :  request.url,
            "index" : index,
            "args": request.args,
            "headers": dict(request.headers),
        }
        if request.method == 'GET' :
                if (index < len(tab_P)):
                        return jsonify(tab_P[index])    #récupère la valeur du tableau à l'index
                else :
                        return jsonify("error : index out of range")
        if request.method == 'DELETE' :
                if (index < len(tab_P)):
                        return jsonify("la valeur "+tab_P.pop(index)+ " a été retirée") #récupère la valeur du tableau à l'index et supprime du tableau
                else :
                        return jsonify("error : index out of range")

#scale
@app.route('/api/scale/', methods=['GET'])
def api_scale():
        
        

        resp = {
            "method":   request.method,
            "url" :  request.url,
            "args": request.args,
            "headers": dict(request.headers),
        }
        if request.method == 'GET' :
                ser.write(b'GET_K')     #envoie à la stm32 que l'on veut faire un GET_K
                tempo = ser.readline().decode() #récupérer la valeur envoyée par la stm32
                return jsonify(tempo[:3])       #retire le \r\n et renvoie la valeur

@app.route('/api/scale/<int:index>', methods=['POST'])
def api_scale_index(index=None):
        resp = {
            "method":   request.method,
            "url" :  request.url,
            "index" : index,
            "args": request.args,
            "headers": dict(request.headers),
        }
        if request.method == 'POST' :
                ser.write(b'SET_K= '+str(index))     #envoie à la stm32 que l'on veut faire un SET_K
                ser.write(index)        #envoie à la stm32 la nouvelle veleur
                return jsonify("le nouveau coeff est "+ str(index))

#angle
@app.route('/api/angle/', methods=['GET'])
def api_angle():
        resp = {
            "method":   request.method,
            "url" :  request.url,
            "args": request.args,
            "headers": dict(request.headers),
        }
        if request.method == 'GET' :
                return jsonify("work in progress")      #angle non récupéré pas eu le temps de le faire