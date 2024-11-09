import json
from flask import Flask, jsonify, abort

app = Flask(__name__)
welcome = "Welcome to 3ESE API!"

@app.route('/api/welcome/')
def api_welcome():
    return welcome

@app.route('/api/welcome/<int:index>')
def api_welcome_index(index):
    # Vérifie si l'index est valide
    if index < 0 or index >= len(welcome):
        abort(404)  # Renvoie une page 404 si l'index est incorrect
    
    # Option 1 : Utiliser json.dumps() pour formater manuellement en JSON
    #return welcome[index]

    # Option 2 : Utiliser json.dumps() et définir Content-Type manuellement
    # return json.dumps({"index": index, "val": welcome[index]}), {"Content-Type": "application/json"}

    # Option 3 : Utiliser jsonify() de Flask pour formater automatiquement
    return jsonify({"index": index, "val": welcome[index]})

@app.errorhandler(404)
def page_not_found(error):
    return render_template('page_not_found.html'), 404

