
from revlens.media.locator import MediaLocator

class AnnBackgroundLocator(MediaLocator):
    '''
    Locator to load a fixed colour or transparent background into a media track
    using input @image:
    @image:eyJ0eXBlIjogImNvbG91ciIsICJyZXNvbHV0aW9uLngiOiAxOTIwLCAicmVzb2x1dGlvbi55IjogMTA4MCwgImNvbG91ciI6ICJ0cmFuc3BhcmVudCJ9
    '''

    def _locate(self, path_input, result):
        
        # self.LOG.debug('_locate(): {}'.format(path_input))
    
        if path_input and path_input.startswith('@image:'):

            result['media.video.component.frames.path'] = path_input
            result['media.video.component.frames.name'] = 'Background'

            return result