

import json
import base64
import smtplib

from email.mime.text import MIMEText
from email.mime.image import MIMEImage
from email.mime.multipart import MIMEMultipart


SENDGRID_API_KEY = ''

EMAIL_SERVICE = None

def send_email_from_msg_via_google(subject, to_addr, msg, from_addr='Revlens Email <justin.ottley@revlens.io>'):

    from email.message import EmailMessage

    from google.auth.transport.requests import Request
    from google.oauth2.credentials import Credentials
    from google_auth_oauthlib.flow import InstalledAppFlow
    from googleapiclient.discovery import build as google_client_api_build

    from rlp import QtCore
    import rlp.util

    SCOPES = ['https://www.googleapis.com/auth/gmail.send']

    global EMAIL_SERVICE
    if not EMAIL_SERVICE:
        creds_raw = rlp.util.AUTH.getCreds('revlens_certificate_token')
        creds_in = json.loads(rlp.util.AUTH.decrypt(creds_raw))

        print(creds_in)
        g_creds = Credentials.from_authorized_user_info(creds_in, SCOPES)

        service = google_client_api_build('gmail', 'v1', credentials=g_creds)
        EMAIL_SERVICE = service


    msg['To'] = to_addr
    msg['From'] = from_addr
    msg['Subject'] = subject

    encoded_message = base64.urlsafe_b64encode(msg.as_bytes()).decode('utf-8')
    
    create_message = {
        'raw': encoded_message
    }
    # pylint: disable=E1101
    send_message = EMAIL_SERVICE.users().messages().send(
        userId="me", body=create_message).execute()

    print(F'Message Id: {send_message["id"]}')



def send_email_from_msg_via_sendgrid(subject, to_addr, msg, from_addr='Revlens Email <justin.ottley@revlens.io>'):
    
    msg['Subject'] = subject
    msg['From'] = from_addr
    msg['To'] = to_addr

    server = smtplib.SMTP_SSL('smtp.sendgrid.net', 465)
    server.ehlo()
    server.login('apikey', SENDGRID_API_KEY)
    server.sendmail(msg['From'], msg['To'], msg.as_string())
    server.close()


def send_email_from_msg(subject, to_addr, msg, from_addr='Revlens Email <justin.ottley@revlens.io>'):
    return send_email_from_msg_via_google(subject, to_addr, msg, from_addr)


def send_email(subject, to_addr, content, format='html', from_addr='Revlens Email <justin.ottley@revlens.io>'):

    msg = MIMEMultipart('alternative')

    msg_text = MIMEText(content, format)

    msg['Subject'] = subject
    msg['From'] = from_addr # 'Revlens Email <justin.ottley@revlens.io>'
    msg['To'] = to_addr

    msg.attach(msg_text)

    server = smtplib.SMTP_SSL('smtp.sendgrid.net', 465)
    server.ehlo()
    server.login('apikey', )
    server.sendmail(msg['From'], msg['To'], msg.as_string())
    server.close()
