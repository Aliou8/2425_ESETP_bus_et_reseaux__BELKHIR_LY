from flask import Flask
from flask import render_template
from flask import request
from flask import jsonify
import serial
import json

app = Flask(__name__)

welcome = "Welcome to 3ESE API!"

@app.route('/api/request/', methods=['GET', 'POST'])
@app.route('/api/request/<path>', methods=['GET','POST'])
def api_request(path=None):
    resp = {
            "method":   request.method,
            "url" :  request.url,
            "path" : path,
            "args": request.args,
            "headers": dict(request.headers),
    }
    if request.method == 'POST':
        resp["POST"] = {
                "data" : request.get_json(),
                }
    return jsonify(resp)


@app.route('/api/welcome/', methods=['GET', 'POST','DELETE'])
def api_welcome():
        global welcome
        resp = {
            "method":   request.method,
            "url" :  request.url,
            "args": request.args,
            "headers": dict(request.headers),
    }
        if request.method == 'POST':
                data = request.get_json()
                if 'message' in data:
                        welcome = data['message']
                        return welcome
        if request.method == 'GET':
                return welcome
        if request.method == 'DELETE':
                welcome =''
                return welcome

@app.route('/api/welcome/<int:index>', methods=['GET', 'POST','PUT','PATCH','DELETE'])
def api_welcome_index(index=None):
        global welcome
        resp = {
            "method":   request.method,
            "url" :  request.url,
            "index" : index,
            "args": request.args,
            "headers": dict(request.headers),
    }
        if request.method == 'POST':
                welcome = request.get_json()
                return welcome
        if request.method == 'GET':
                if index == None:
                        return welcome
                elif index > 19 or index < 0 :
                        return abort(404)
                else:
                        return jsonify({"index": index, "val": welcome[index]})
        if request.method =='PUT':
                data = request.get_json()
                if 'word' in data:
                        word_insert = data['word']
                        if 0 <= index <= len(welcome):
                                welcome = welcome[:index]+word_insert+welcome[index:]
                                return welcome
        if request.method =='PATCH':
                data = request.get_json()
                if 'letter' in data:
                        letter_insert = data['letter']
                        if 0 <= index <= len(welcome):
                                welcome = welcome[:index]+letter_insert+welcome[index+1:]
                                return welcome
        if request.method =='DELETE':
                if 0 <= index <= len(welcome):
                        welcome = welcome[:index]+welcome[index+1:]
                        return welcome


@app.errorhandler(404)
def page_not_found(error):
    return render_template('page_not_found.html'), 404
