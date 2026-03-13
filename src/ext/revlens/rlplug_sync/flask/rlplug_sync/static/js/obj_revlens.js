
class RevlensWebSocketHandler {

    constructor(ws_url) {

        var me = this;

        this.url = "ws://" + ws_url;
        this.websocket = new WebSocket(this.url);
        this.websocket.onopen = function(result) {

            console.log('websocket opened');
            me.callFromClient('get_properties');
            me.callFromClient('get_rooms');

        }

        this.websocket.onmessage = function(result) {
            console.log('got a message');
            console.log(result);

            var msg = JSON.parse(result.data);
            console.log(msg);

            window.room_view.on_message_received(msg);
        }
    }

    callFromClient(method, args, kwargs, callback) {

        console.log('callFromClient(): ' + method);

        if (args == undefined) {
            args = [];
        }

        if (kwargs == undefined) {
            kwargs = {};
        }


        console.log(args);
        console.log(kwargs);

        this.websocket.send(JSON.stringify(
            {
                'method': 'callFromClient',
                'args': [],
                'kwargs': {
                    'method': method,
                    'args': args,
                    'kwargs': kwargs
                }
            }
        ));
    }
}


class RevlensDirectHandler {

    constructor()
    {

        window.revlens_room_controller.sendCallToClient.connect(this.onRemoteCallReceived)
        console.log('sendCallToClient signal connected');

    }

    onRemoteCallReceived(payload)
    {
        console.log('onRemoteCallReceived');
        console.log(payload);

        window.room_view.on_message_received(payload);
    }


    callFromClient(method, args, kwargs)
    {

        console.log('RevlensDirectHandler: callFromClient(): ' + method);

        if (args == undefined) {
            args = [];
        }   

        if (kwargs == undefined) {
            kwargs = {};
        }


        console.log(args);
        console.log(kwargs);
        
        window.revlens_room_controller.callFromClient(
            method, args, kwargs
        );
    }
}


class RevlensCommsController {

    constructor(comm_style) {

        if (comm_style == undefined) {
            comm_style = 'websocket';
        }

        if (comm_style == 'direct') {
            this._c = new RevlensDirectHandler();
        }
    }


    callFromClient(method, args, kwargs, callback) {
        this._c.callFromClient(method, args, kwargs, callback);
    }
}