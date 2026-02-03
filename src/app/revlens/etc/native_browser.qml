import QtQuick 2.0
import QtWebView 1.1
import QtQuick.Controls 1.1

Rectangle {
    width: 300
    height: 1200
    color: "red"
    
    WebView {
        id: webView
        anchors.fill: parent
        url: loadUrl
        onLoadingChanged: {
            if (loadRequest.errorString)
            console.error(loadRequest.errorString);
        }
    }
}

