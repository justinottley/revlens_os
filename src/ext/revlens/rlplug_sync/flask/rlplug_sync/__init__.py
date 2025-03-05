
from flask import Flask

def create_app(*args, **kwargs):

    from . import app
    return app.FLASK_APP
