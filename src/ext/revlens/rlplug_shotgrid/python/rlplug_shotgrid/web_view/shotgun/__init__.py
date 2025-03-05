#
# Copyright 2024 Justin Ottley
#
# Licensed under the terms set forth in the LICENSE.txt file
#

import os
import json
import pprint
import logging
import traceback

import rlp.util.config

import revlens.cmds
import revlens.media


from rlplug_shotgrid.sg_handler import ShotgunHandler
# from rlplug_shotgrid.locator import ShotgunRemoteFileLocator

from .shotgun_js import SHOTGUN_JS_INJECTION_SCRIPT
from .css import CSS_TEMPLATE


LOG = logging.getLogger('rlp.{}'.format(__name__))


def _get_site_config_path():

    if os.getenv('REVLENS_SHOTGRID_CONFIG_PATH'):
        return os.getenv('REVLENS_SHOTGRID_CONFIG_PATH')


    site_config_dir = os.path.join(
        rlp.util.config.CONFIG_DIR,
        'shotgrid'
    )

    if not os.path.isdir(site_config_dir):
        os.makedirs(site_config_dir)

    site_config_path = os.path.join(
        site_config_dir,
        'shotgrid_site.json'
    )

    return site_config_path


def write_site_config(site_config):

    config_path = _get_site_config_path()
    with open(config_path, 'w') as wfh:
        wfh.write(json.dumps(site_config, indent=2))

        LOG.info('Wrote {}'.format(config_path))


def read_site_config():

    config_path = _get_site_config_path()
    if os.path.isfile(config_path):
        LOG.debug('Reading ShotGrid config: {}'.format(config_path))
        with open(config_path) as fh:
            site_config = json.load(fh)
            return site_config
        
    else:
        LOG.error('ShotGrid config file not found: {}'.format(config_path))


def init_sg_revlens(site_config=None, force=False):

    if not force and ShotgunHandler.SG_URL != None:
        LOG.warning('ShotGrid API already initialized, skipping')
        return

    if site_config is None:
        site_config = read_site_config()

    if not site_config:
        LOG.error('Error: No ShotGrid site config, aborting')
        return

    ShotgunHandler.SG_URL = site_config['sg_url']
    ShotgunHandler.SG_API_USER = site_config['api_user']
    ShotgunHandler.SG_API_KEY = site_config['api_key']

    # sg_remote_file_locator = ShotgunRemoteFileLocator()
    # revlens.media.LOCATORS.append(sg_remote_file_locator)


def _get_sftset_dir():
    sftset_dir = os.path.dirname(__file__)
    for x in range(6):
        sftset_dir = os.path.dirname(sftset_dir)
        
    return sftset_dir


def _get_jquery_dir():
    return os.path.join(_get_sftset_dir(), 'etc', 'jqueryui', '1.11.1')
    
def _get_jqueryui_dir():
    return os.path.join(_get_sftset_dir(), 'etc', 'jqueryui', 'dark_hive', 'inst')


def init_shotgun_plugin(web_view):

    print('SHOTGUN BROWSER CREATE')

    jquery_src_dir = _get_jquery_dir()
    jquery_ui_src_dir = _get_jqueryui_dir()

    contents_all = ''

    jquery_file_path = os.path.join(jquery_src_dir, 'js', 'jquery.js')
    LOG.info(jquery_file_path)

    with open(jquery_file_path) as fh:

        jcontents_all = ''.join(fh.readlines())
        # jcontents_all += '\nvar jq = jQuery.noConflict();\n'
        web_view.addScript('jquery.js', jcontents_all, 1)


    for file_entry in ['jquery-ui.min.js']:
        jquery_file_path = os.path.join(jquery_ui_src_dir, file_entry)

        LOG.info(jquery_file_path)
        with open(jquery_file_path) as fh:
            contents_all = ''.join(fh.readlines())
            
            '''
            if file_entry == 'jquery.js':
                # contents_all += '\nvar jq = jQuery.noConflict();\n'
                # contents_all += "\nvar qt = { 'jQuery': jQuery.noConflict(true) };"
            '''



    for css_file in ['jquery-ui.min.css', 'jquery-ui.theme.min.css']: 

        jquery_css_file_path = os.path.join(jquery_ui_src_dir, css_file)

        LOG.info(jquery_css_file_path)
        with open(jquery_css_file_path) as fh:
            
            css_contents = ''.join(fh.readlines())
            css_js = CSS_TEMPLATE.format(css_contents=css_contents)
            web_view.addScript(css_file, css_js, 1)
            
    
    # LOG.info(contents_all)
    
    
    script_contents_all = contents_all + SHOTGUN_JS_INJECTION_SCRIPT
    # script_contents_all = SHOTGUN_JS_INJECTION_SCRIPT
    
    web_view.setPostLoadScript("RevlensShotgunIntegration", script_contents_all)
    # web_view.setWebChannelPyHandler(
    #     'revlens',
    #     'rlplug_shotgrid.sg_handler',
    #     'ShotgunHandler')

    # web_view.setUrl(sg_url)

