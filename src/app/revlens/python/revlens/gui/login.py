
import logging

import revlens

LOG = logging.getLogger('rlp.{}'.format(__name__))


def login_done(result):

    LOG.info('revlens: login done: {}'.format(result))

    if not result.get('connect_state') or result.get('connect_state') != 'OK':
        revlens.CNTRL.loginCntrl().emitLoginFailed()
        return


    from rlp.core.net.websocket import RlpClient

    rlpClient = RlpClient.instance()

    if rlpClient is None:
        print('WARNING: no Site Client instance!')
        return


    LOG.info('revlens: login done on "{}" - {} url: "{}" site_name: {}'.format(
        rlpClient.url,
        rlpClient,
        rlpClient._wsclient.url().toString().toStdString(),
        rlpClient.site_name
    ))

    # TODO FIXME
    # ???? WHY IS THIS NOT SET FOR WASM?
    #
    # ws_client_url = rlpClient._wsclient.url().toString()
    # if not ws_client_url:
    #     rlpClient._wsclient.setUrl(SiteWsClient.instance().url)

    lresult = {'result': False}
    if rlpClient and rlpClient.auth_key:
        lresult = rlpClient.info
        lresult['result'] = True

    revlens.CNTRL.loginCntrl().loginDone(lresult)

