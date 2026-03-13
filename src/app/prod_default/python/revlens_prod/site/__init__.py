
import logging

G_PANEL = None

LOG = logging.getLogger(__name__)

def login(site_hostinfo, username, password, callback=None):

    LOG.debug('Attempting login: {} {} cb: {}'.format(site_hostinfo, username, callback))

    # NOTE: Late import, requires non-standalone libs
    from ..client_ws import SiteWsClient

    client = SiteWsClient.init(site_hostinfo)
    return client.login(username, password, callback)

