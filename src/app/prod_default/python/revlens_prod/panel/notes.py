
import os
import uuid
import time
import base64
import pprint
import logging
import tempfile
import functools
import traceback

try:
    from email.mime.text import MIMEText
    from email.mime.image import MIMEImage
    from email.mime.multipart import MIMEMultipart

    from rlp.core import rlp_email

except:
    print('WARNING: email imports failed')
    print(traceback.format_exc())

from rlp.Qt import QStr, QV
from rlp import QtCore, QtGui


import rlp.core as RlpCore
import rlp.util

import rlp.gui as RlpGui
from rlp.gui.dialogs import ConfirmDialog
from rlp.gui.userlabel import UserLabel

import revlens
import revlens.cmds

import rlplug_sync
from rlplug_qtbuiltin import RLQTMEDIA_PreviewMakerNode

from revlens_prod.client_ws import SiteWsClient

_PANELS = []


class SendEmailDialog(ConfirmDialog):

    def _init_layout(self):

        self.email_layout = RlpGui.GUI_HLayout(self)

        self.email_label = RlpGui.GUI_Label(self, "Recipients: ")
        self.email_text = RlpGui.GUI_TextEdit(self, 260, 60)

        self.email_layout.addSpacer(20)
        self.email_layout.addItem(self.email_label, 0)
        self.email_layout.addItem(self.email_text, 0)

        self.layout.addSpacer(100)
        self.layout.addItem(self.email_layout, 0)


    @property
    def value(self):
        return self.email_text.text()


class NoteHTMLExporter:

    app_rootpath = rlp.util.APPGLOBALS.globals()['app.rootpath']

    _uimage = RlpCore.UTIL_Image()

    frame_thumb_b64_png_data = '/9j/4AAQSkZJRgABAQEAYABgAAD/2wBDAAgGBgcGBQgHBwcJCQgKDBQNDAsLDBkSEw8UHRofHh0aHBwgJC4nICIsIxwcKDcpLDAxNDQ0Hyc5PTgyPC4zNDL/2wBDAQkJCQwLDBgNDRgyIRwhMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjIyMjL/wAARCAAUABQDASIAAhEBAxEB/8QAHwAAAQUBAQEBAQEAAAAAAAAAAAECAwQFBgcICQoL/8QAtRAAAgEDAwIEAwUFBAQAAAF9AQIDAAQRBRIhMUEGE1FhByJxFDKBkaEII0KxwRVS0fAkM2JyggkKFhcYGRolJicoKSo0NTY3ODk6Q0RFRkdISUpTVFVWV1hZWmNkZWZnaGlqc3R1dnd4eXqDhIWGh4iJipKTlJWWl5iZmqKjpKWmp6ipqrKztLW2t7i5usLDxMXGx8jJytLT1NXW19jZ2uHi4+Tl5ufo6erx8vP09fb3+Pn6/8QAHwEAAwEBAQEBAQEBAQAAAAAAAAECAwQFBgcICQoL/8QAtREAAgECBAQDBAcFBAQAAQJ3AAECAxEEBSExBhJBUQdhcRMiMoEIFEKRobHBCSMzUvAVYnLRChYkNOEl8RcYGRomJygpKjU2Nzg5OkNERUZHSElKU1RVVldYWVpjZGVmZ2hpanN0dXZ3eHl6goOEhYaHiImKkpOUlZaXmJmaoqOkpaanqKmqsrO0tba3uLm6wsPExcbHyMnK0tPU1dbX2Nna4uPk5ebn6Onq8vP09fb3+Pn6/9oADAMBAAIRAxEAPwDyrUYrHSxMDZ2WEleKBXjkd5NmASxDgDr6Vj/2tD/0B9O/75k/+Lo8RE/8JFqAzx9of+dUra1nvJhDbxNJIRnaPSgDtdA8P6Z4h037bNbC3cOYysDMFOMHOCT6/pRW54Ot0tNKuLeOUSrHdOu8dDwtFAHnh125c7pYLKV+8klqjM3uTjk0n9ty/wDPnp3/AIBx/wCFFFAC/wDCQ6moCwTi3QdI4EWNfrgDrRRRQB//2Q=='

    header_markup = '<html><body style="font-family: sans-serif;background-color: #282828;">'
    footer_markup = '</body></html>'

    title_template = '''
    <div style="clear: both;">&nbsp;</div>
    <div style="width: 400px; align:center; padding: 10px; color:#FFFFFF; margin-left:1%">
        <center><img src="{REVLENS_IMG_DATA}" width="140" /></center>
    </div>
    <div style="width: 400px; align:center; padding: 10px; color:#FFFFFF; margin-left:1%">
        <center><h1>{TITLE}</h1></center>
    </div>
    '''

    note_template = '''
    <div style="width: 400px; padding: 10px; color:#FFFFFF; background-color: #3C3C3C; margin-left:1%;">
        <div>
            <span style="float:left;width:230px;margin-top:10px;">
                <div style="font-size: 1.6em">{USER_FULLNAME}</div>
                <div style="font-size: 0.9em">{TIMESTAMP}</div>
            </span>
            <span style="float:left; padding-top:10px;">
                <img src="{THUMB_DATA}" />
            </span>
        </div>
        <div style="clear: both; padding-top:10px;"></div>
        <div style="clear:both; font-size: 0.9em;">
            <span style="margin-top:3px; font-weight: bold;">{MEDIA_NAME}</span>
        </div>
        <div style="clear: both; padding-top:3px;"></div>
        <div style="clear:both; font-size: 0.9em;">
                <span style="float:left;"><img src="{FRAME_THUMB_DATA}" /></span>
                <span style="float:left;margin-top:3px;">&nbsp;&nbsp;{FRAME}</span>
        </div>
        <div style="clear: both;"><p>&nbsp;</p></div>
        <div style="clear:both;">
            <div>{MESSAGE}</div>
        </div>
    </div>
    '''

    spacer_template = '<div style="clear: both;">&nbsp;</div>'

    def __init__(self):
        pass


    def export_note(self, note):

        img_b64data = self._uimage.toBase64(note.thumb, "JPG")
        user_fullname = note.username_label.name_str
        note_timestamp = note.create_time_str

        note_markup = self.note_template.format(
            USER_FULLNAME=user_fullname,
            TIMESTAMP=note_timestamp,
            MEDIA_NAME=note.media_info['media.name'],
            THUMB_DATA='{THUMB_DATA}',
            FRAME_THUMB_DATA='{FRAME_THUMB_DATA}',
            FRAME=int(note.frame),
            MESSAGE=note.text
        )

        result = {
            'markup': note_markup,
            'thumb_data': img_b64data
        }

        return result


    def export(self, note_list):

        for note in note_list:

            markup += self.export_note(note)
            markup += self.spacer_template

        markup += '</body></html>'
        return markup


class NoteHTMLEmailExporter(NoteHTMLExporter):

    def __init__(self):
        super().__init__()

        self.msg_images = []

        self.frame_thumb_msg = MIMEImage(base64.b64decode(self.frame_thumb_b64_png_data), 'png')
        self.frame_thumb_msg.add_header('Content-Id', '<frame_thumb>')
        self.msg_images.append(self.frame_thumb_msg)

    def export_note(self, note):

        eresult = NoteHTMLExporter.export_note(self, note)

        markup = eresult['markup']
        img_b64data = eresult['thumb_data']

        img_id = str(uuid.uuid4())

        thumb_cid = 'cid:{}'.format(img_id)
        thumb_frame_cid = 'cid:frame_thumb'

        # thumb_b64 = 'data:image/jpg;base64,{}'.format(img_b64data)

        markup = markup.format(THUMB_DATA=thumb_cid, FRAME_THUMB_DATA=thumb_frame_cid)


        msg_img = MIMEImage(base64.b64decode(img_b64data), 'jpeg')
        msg_img.add_header('Content-Id', '<{}>'.format(img_id))
        msg_img.add_header('Content-Disposition', 'inline', filename=img_id)

        self.msg_images.append(msg_img)

        return markup


    def export(self, title, note_list):
        try:
            return self._export(title, note_list)

        except:
            print(traceback.format_exc())

    def _export(self, title, note_list):
        
        revlens_logo_path = os.path.join(self.app_rootpath, 'gui', 'static', 'icon_files', 'revlens_logo_alpha.png')
        revlens_logo_data = open(revlens_logo_path, 'rb').read()

        title_markup = self.title_template.format(
            TITLE=title,
            REVLENS_IMG_DATA='cid:revlens_logo'
        )

        msg_logo = MIMEImage(revlens_logo_data, 'png')
        msg_logo.add_header('Content-Id', '<revlens_logo>')
        msg_logo.add_header('Content-Disposition', 'inline', filename='revlens_logo')

        note_markup = ''
        for note in note_list:

            note_markup += self.export_note(note)
            note_markup += self.spacer_template

        markup = self.header_markup + title_markup + note_markup + self.footer_markup
        msg_text = MIMEText(markup, 'html')

        msg = MIMEMultipart()
        msg.attach(msg_text)

        msg.attach(msg_logo)

        for msg_img in self.msg_images:
            msg.attach(msg_img)

        return msg


class Note(RlpGui.GUI_ItemBase):

    NOTE_HEIGHT = 190

    _uimage = RlpCore.UTIL_Image()

    def __init__(self, parent, username, user_info, text, media_info, media, create_time=None, thumb_data=None):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self.thumb = None
        self.thumb_path = None
        self.thumb_data = thumb_data # serialized base64 thumb data and size

        self.username = username
        self.user_info = user_info
        self.text = text
        self.media_info = media_info
        self.frame = self.media_info['media.frame'] # frame

        self.media = media
        
        self.create_time = create_time or time.time()
        self.create_time_str = time.strftime('%Y-%m-%d %H:%M:%S', time.localtime(self.create_time))
        
        
        self.setItemHeight(self.NOTE_HEIGHT)
        self.setItemWidth(parent.width())


        self.note_text = RlpGui.GUI_TextEdit(self, 200, 110)
        self.note_text.textArea().setReadOnly(True)
        self.note_text.setPos(5, 110)

        self.note_text.setWordWrap(True)
        self.note_text.setOutlined(False)
        self.note_text.setHScrollBarVisible(False)
        self.note_text.setText(self.text)

        self.username_label = UserLabel(self,
            self.username,
            self.user_info.get('first_name', 'Unknown'),
            self.user_info.get('last_name', 'User')
        )

        self.username_label.onParentSizeChangedItem(self.width(), 0)
        self.username_label.setPos(10, 40)

        # self.layout.addItem(self.text_label)
        # self.layout.addItem(self.username_label)

        self.frame_button = RlpGui.GUI_SvgButton(
            ':misc/frame.svg', self, 20)

        self.frame_button.setOutlined(False)
        self.frame_button.setOverlayText(str(int(self.frame)))
        self.frame_button.setOverlayTextPos(25, 9)
        self.frame_button.setToolTipText(
            "Media Frame: {} Session Frame: {}".format(
            int(self.frame),
            int(self.media_info['session.frame']))
        )


        self.frame_button.icon().setBgColour(QtGui.QColor(120, 120, 120))
        self.frame_button.icon().setFgColour(QtGui.QColor(250, 250, 250))
        self.frame_button.buttonPressed.connect(self.on_frame_button_pressed)

        self.close_button = RlpGui.GUI_SvgButton(
            ":/feather/lightgrey/x-circle.svg",
            self, 20
        )
        self.close_button.icon().setBgColour(QtGui.QColor(120, 120, 120))
        self.close_button.icon().setFgColour(QtGui.QColor(250, 250, 250))
        
        self.close_button.setPos(parent.width() - 70, 10)

        self.toolbar_layout = RlpGui.GUI_HLayout(self)
        self.toolbar_layout.addItem(self.frame_button, 0)

        self.toolbar_layout.setPos(10, 10)

        self._generate_thumbnail(parent.grab_annotations)

        self.onParentSizeChangedItem(parent.width(), parent.height())


    def _init_thumb(self, thumb_img):

        # QtCore.Qt.SmoothTransformation compiled in default in binding
        self.thumb = thumb_img.scaledToHeight(70)
        self.thumb_data = self._uimage.toBase64(
            self.thumb, "JPG")

    @property
    def disp(self):
        return revlens.CNTRL.getVideoManager().getDisplayByIdx(0)


    def _generate_thumbnail(self, do_grab_annotations=True):

        if self.thumb_data:
            self.thumb = self._uimage.fromBase64(self.thumb_data)

        elif do_grab_annotations:
            self.disp.grabImage() # asyncronous, will land when slot comes back

        elif self.media:
            preview_node = RLQTMEDIA_PreviewMakerNode(self.media)
            image = preview_node.makePreview(int(self.media_info['session.frame']))

            self._init_thumb(image)

        else:
            print('WARNING: SKIPPING THUMBNAIL GEN, NO MEDIA NODE, BAKE THUMBNAIL DATA INTO NOTE')


    def _setWidth(self, width):
        
        self.setItemWidth(width)

        max_text_width = width - 40 # self.thumb.width() - 30

        self.note_text.setItemWidth(max_text_width)
        self.note_text.setText(self.note_text.text())
        self.note_text.resetPosition()

        self.close_button.setPos(width - 70, 7)


    def get_payload(self):
        return {
            'text': self.text,
            'frame': self.frame,
            'username': self.username,
            'first_name': self.user_info.get('first_name', '-'),
            'last_name': self.user_info.get('last_name', '-'),
            'create_time': self.create_time,
            'thumb_data': self.thumb_data
        }


    def onParentSizeChangedItem(self, width, height):
        self._setWidth(width)


    def on_frame_button_pressed(self, md=None):
        revlens.cmds.goto_frame(int(self.media_info['session.frame']) - 1)


    def boundingRect(self):
        result = QtCore.QRectF(
            1, 0, self.width() - 40, self.height() - 1)

        return result

    def paintItem(self, painter):

        p = QtGui.QPen()
        p.setColor(QtGui.QColor(0, 0, 0))

        b = QtGui.QBrush(QtGui.QColor(60, 60, 60))

        painter.setPen(p)
        painter.setBrush(b)

        painter.drawRect(self.boundingRect())

        if self.thumb:
            painter.drawImage(
                int(self.width() - 40 - self.thumb.width() - 5),
                35,
                self.thumb
            )


        p = painter.pen()
        p.setColor(QtGui.QColor(250, 250, 250))

        painter.save()

        f = painter.font()
        f.setPixelSize(10)
        painter.setFont(f)
        painter.setPen(p)
        painter.drawText(
            55, 78, QStr(self.create_time_str)
        )

        painter.restore()



class NotesPanel(RlpGui.GUI_ItemBase):

    def __init__(self, parent):
        RlpGui.GUI_ItemBase.__init__(self, parent)

        self._destructed = False

        self.LOG = logging.getLogger('rlp.{}.{}'.format(self.__class__.__module__, self.__class__.__name__))

        self.notes = []

        self.new_note_textarea = RlpGui.GUI_TextEdit(self, 200, 60)
        self.new_note_textarea.setTempHintText('Write a note..')
        self.new_note_textarea.enterPressed.connect(self.new_note_requested)

        self.toolbar_layout = RlpGui.GUI_HLayout(self)

        self.refresh_button = RlpGui.GUI_SvgButton(
            ":feather/lightgrey/refresh-cw.svg",
            self,
            20
        )
        self.refresh_button.setToolTipText("Refresh Notes")

        self.export_button = RlpGui.GUI_SvgButton(
            ":feather/lightgrey/mail.svg",
            self,
            20
        )

        self.annotation_button = RlpGui.GUI_SvgButton(
            ':note.svg',
            self,
            20
        )
        self.annotation_button.icon().setOutlined(True)
        self.annotation_button.icon().setBgColour(QtGui.QColor(180, 180, 180))
        self.annotation_button.setMetadata("toggled", True)
        self.annotation_button.buttonPressed.connect(self.on_toggle_grab_annotations)
        self.annotation_button.setToolTipText("Show Annotation in Thumbnail")
        self.grab_annotations = True

        self.export_button.setToolTipText("Email Notes")
        self.export_button.buttonPressed.connect(self.on_export_requested)

        self.toolbar_layout.addItem(self.refresh_button, 3)
        self.toolbar_layout.addSpacer(10)
        self.toolbar_layout.addItem(self.export_button, 3)
        self.toolbar_layout.addDivider(20, 25)

        self.toolbar_layout.addItem(self.annotation_button, 3)

        self.layout = RlpGui.GUI_VLayout(self)
        self.layout.setHSpacing(10)

        self.layout.addItem(self.toolbar_layout, 0)
        self.layout.addSpacer(10)
        self.layout.addItem(self.new_note_textarea, 0)
        self.layout.addSpacer(15)

        self.note_list_sa = RlpGui.GUI_ScrollArea(self)
        self.note_list_sa.setOutlined(False)

        self.note_layout = RlpGui.GUI_VLayout(self.note_list_sa.content())

        self.note_list_sa.setPos(10, 100)

        self._init_connections()
        self._init_notes_from_session()

        self.onParentSizeChangedItem(parent.width(), parent.height())


    @property
    def edbc(self):
        return SiteWsClient.instance()

    def _init_connections(self):

        # revlens.CNTRL.getVideoManager().frameDisplayed.connect(self.on_frame_displayed)
        # revlens.CNTRL.sessionChanged.connect(self.on_session_changed)

        session = revlens.CNTRL.session()
        session.sessionCleared.connect(self.clear)
        session.mediaAddedDict.connect(self._init_notes_from_media)

        # Only one display supported
        vmgr = revlens.CNTRL.getVideoManager()
        if vmgr.getDisplayCount() > 0:
            disp = vmgr.getDisplayByIdx(0)
            disp.grabImageReady.connect(self.on_grab_image_ready)


    def clear(self):

        print('NOTES: CLEAR!!!')

        if self._destructed:
            return

        self.note_layout.clear()
        self.notes = []


    def destruct(self):
        print('DESTRUCT!!')
        self._destructed = True


    def onParentSizeChangedItem(self, width, height):

        self.setWidth(width - 20)
        self.setHeight(height)

        self.new_note_textarea.setWidth(width - 20)

        note_height = (len(self.notes) * Note.NOTE_HEIGHT)
        sa_ypos = self.note_list_sa.pos().y()

        self.note_list_sa.setMaxChildHeight(note_height)
        self.note_list_sa.setHeight(height - sa_ypos)


    def _init_notes_from_media(self, media_in):

        if 'Node' in media_in.__class__.__name__:
            props = media_in.getProperties()

        else:
            props = media_in['node_properties']


        print('_init_notes_from_media()')
        print(props)


        if 'media.notes' not in props:
            print('NO NOTES FOUND IN PROPERTIES')
            return

        note_list = props.get('media.notes', [])

        for note_entry in note_list:
            
            if type(note_entry) != dict:
                print('Skipping note entry {}, invalid'.format(note_entry))
                continue
            
            frame = note_entry['frame']
            media_info = {
                'session.frame': int(frame),
                'media.frame': frame - props['session.frame'],
                'media.name': props['media.name'],
                'media.uuid': props['graph.uuid']
            }
            self.add_note(
                media_info,
                note_entry['text'],
                None,
                note_entry['create_time'],
                note_entry['username'],
                {
                    'first_name': note_entry['first_name'],
                    'last_name': note_entry['last_name']
                },
                note_entry.get('thumb_data')
            )


    def _init_notes_from_session(self, session_info=None):

        self.clear()

        session = revlens.CNTRL.session()

        for track_idx in range(session.numTracks()):
            track = session.getTrackByIndex(track_idx)
            for media_uuid in track.getNodeInfo().keys():

                media = session.getNodeByUuid(media_uuid)
                if not media:
                    print('ERROR: INVALID MEDIA: {}'.format(media_uuid))
                    continue

                self._init_notes_from_media(media)



    def on_session_changed(self, session):

        session.sessionCleared.connect(self.clear)
        self._init_notes_from_session()


    def _sync_note(self, media_props, media_notes):

        track_uuid = media_props['session.track'].uuid().toString()
        media_uuid = media_props['graph.uuid'].toString()

        print('sync_note track: {} media: {} notes: {}'.format(
            track_uuid, media_uuid, media_notes
        ))

        kwargs = {
            'update_info': {
                'media_type': 'media_property',
                'track_uuid': track_uuid,
                'media_uuid': media_uuid,
                'prop_key': 'media.notes',
                'prop_val': media_notes
            }
        }

        revlens.CNTRL.emitSyncAction(
            'mediaUpdate',
            kwargs
        )


    def add_note(self, media_info, note_text, media, create_time=None, username=None, user_info=None, thumb_data=None):

        if not username and self.edbc:
            username = self.edbc.username

        if not user_info and self.edbc:
            user_info = self.edbc.user_info

        if username is None:
            username = 'unknown'

        if user_info is None:
            user_info = {}

        note = Note(
            self,
            username,
            user_info,
            note_text,
            media_info,
            media,
            create_time=create_time,
            thumb_data=thumb_data
        )

        note.close_button.buttonPressed.connect(functools.partial(
            self.on_remove_note,
            note
        ))

        self.notes.append(note)
        self.note_layout.addItem(note, 0)

        note_height = (len(self.notes) * Note.NOTE_HEIGHT) + 100
        self.note_list_sa.setMaxChildHeight(note_height)
        self.note_layout.setItemHeight(note_height)

        return note


    def new_note_requested(self, val):

        note_text = self.new_note_textarea.text()
        frame = revlens.cmds.get_current_frame_num() + 1# self.frame_label.text()
        
        self.new_note_textarea.clear()

        media = revlens.CNTRL.session().getNodeByFrame(int(frame))
        if not media: # .isValid():

            print('ERROR INVALID MEDIA AT FRAME {}'.format(frame))
            return

        props = media.getProperties()
        media_info = {
            'session.frame': int(frame),
            'media.frame': frame - props['session.frame'],
            'media.name': props['media.name'],
            'media.uuid': props['graph.uuid']
        }
        # pprint.pprint(props)

        curr_notes = []
        if 'media.notes' in props:
            curr_notes = list(props['media.notes'])

        # if frame not in notes:
        #     notes[frame] = []

        # curr_notes = list(notes[frame])

        print('NOTES: {}'.format(pprint.pformat(curr_notes)))

        new_note = self.add_note(media_info, note_text, media)
        curr_notes.append(new_note.get_payload())

        media.insertProperty('media.notes', curr_notes)

        self._sync_note(props, curr_notes)


    def on_export_requested(self, md=None):
        print('EXPORT')

        room_name = rlplug_sync.RLSYNC_ROOMCONTROLLER_OBJ.roomName()
        if not room_name:
            room_name = revlens.CNTRL.session().name()

        curr_time = time.ctime(time.time())
        subject = '{} - {}'.format(room_name, curr_time)

        # floating_pane = revlens.VIEW.createFloatingPane(400, 220, False)
        diag = SendEmailDialog.create(
            'Email Notes',
            subject,
            'Send Email',
            400,
            280,
            'mail'
        )

        def _handle_dialog_done(md):
            print('DONE')
            print(md)
            do_send = md.get('value')
            if do_send:
                to_addr = md['value']

                ne = NoteHTMLEmailExporter()
                msg = ne.export(subject, self.notes)
                
                try:
                    rlp_email.send_email_from_msg(subject, to_addr, msg)
                except:
                    print(traceback.format_exc())


        diag.accept.connect(_handle_dialog_done)
        
        
        '''
        
        
        note_output_path = '/tmp/note_output.html'
        with open(note_output_path, 'w') as wfh:
            wfh.write(markup)


        print('Wrote {}'.format(note_output_path))
        '''

    def on_remove_note(self, note, md=None):

        diag = ConfirmDialog.create(
            'Delete Note', 'Delete Note - are you sure?', 'Delete')
        
        def _on_delete(md):
            self.notes.remove(note)
            self.note_layout.removeItem(note)
            self.note_layout.update()

        diag.accept.connect(_on_delete)


    def on_grab_image_ready(self, img):
        if not self.notes:
            return
        self.notes[-1]._init_thumb(img)
        self.notes[-1].update()


    def on_toggle_grab_annotations(self, md):
        if md['toggled']:
            self.annotation_button.icon().setOutlined(False)
            self.annotation_button.setMetadata("toggled", False)
            self.annotation_button.icon().setBgColour(QtGui.QColor(80, 80, 80))
            self.grab_annotations = False

        else:
            self.annotation_button.icon().setOutlined(True)
            self.annotation_button.setMetadata("toggled", True)
            self.annotation_button.icon().setBgColour(QtGui.QColor(180, 180, 180))
            self.grab_annotations = True


        self.annotation_button.icon().update()


def create(parent):
    print('CREATE!!!!!!!!!!!!! {}'.format(parent))
    notes_panel = NotesPanel(parent)

    # global _PANELS
    # _PANELS.append(notes_panel)

    return notes_panel