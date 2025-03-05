

from flask import Flask, jsonify, render_template

FLASK_APP_NAME = 'rlplug_sync'
FLASK_APP = Flask(FLASK_APP_NAME)


@FLASK_APP.route('/', methods=['GET'])
def test_endpoint():
    msg = '<html><head></head><body>Hello World 1</body></html>'
    return msg

@FLASK_APP.route('/default/index', methods=['GET'])
def default_index():
    return render_template('index.html', data={'comm':'direct'})
