

class RoomListWidget extends SSObject {

    constructor(selector_parent, options) {

        var selector = selector_parent + '_' + options['room_dirname'];
        super(selector, options);

        this.selector_parent = selector_parent;
        this.selector_wrapper = this.selector + '_wrapper';
        this.selector_load_button = selector + '_load_button';
        this.selector_delete_button = selector + '_delete_button';

    }


    init() {
        console.log('RoomListWidget init()');

        var markup = this.format_markup_init();
        $(this.selector_parent).append(markup);

        this.load_button = new Button(
            this.selector_load_button,
            {
                'name': 'join',
                'label': 'Join'
            }
        );

        this.load_button.connect('clicked', this, this.on_load_clicked);

        this.delete_button = new Button(
            this.selector_delete_button,
            {
                'name': 'Delete',
                'label': 'Delete'
            }
        );

        this.delete_button.connect('clicked', this, this.on_delete_clicked);
        
    }


    format_markup_init() {
        
        var markup = '<div class="ui-widget-header ui-corner-all room-item" id="' + this.selector.substring(1) + '"></div>';
        return markup;
    }

    format_markup(session_info) {

        console.log('RoomView format_markup()');
        console.log(session_info);

        if (session_info == undefined) {
            session_info = {}
        }

        if (session_info['state'] == undefined) {
            session_info['state'] = 'unknown';
        }

        var status_icon_src = ''; // /rlplug_review/static/images/Antu_waiting.svg.png';
        if (session_info['state'] == 'complete') {
            status_icon_src = '/rlplug_review/static/images/if_tick_48879.png';
        }
        var markup = '';
        markup += '<div class="room-item" id="' + this.selector_wrapper.substring(1) + ' margin-top: 10px;">';
        markup += '<div><span style="float: left;"><img width="32" src="' + status_icon_src + '"/></span>';
        markup += '<span>' + this.options['room_name'] + '</span>';
        markup += '</div>';
        // markup += '<div style="clear: both;"></div>';
        markup += '<div style="margin-left:20px; font-size: 0.8em;">' + this.options['created_at_str'] + '</div>';
        markup += '<div>';
        markup += '  <div style="float: left; margin-top: 5px;" id="' + this.selector_load_button.substring(1) + '"></div>';
        markup += '  <div style="float: right; margin-top: 5px;" id="' + this.selector_delete_button.substring(1) + '"></div>';
        markup += '</div>'
        markup += '<div style="clear: both;"></div>';
        markup += '</div>';
        return markup;
    }


    _refresh(result) {
        /*
        result = {
            'state': 'complete'
        }
        */

        var markup = this.format_markup(result);
        $(this.selector).html(markup);

        
        this.load_button.init();
        this.delete_button.init();
    }


    refresh() {

        var me = this;

        console.log('RoomView: refresh()')

        console.log(this.options);
        if ((this.options['metadata'] == undefined) || (this.options['metadata']['farm_session_id'] == undefined)) {
            console.log('cannot get farm status, no metadata / farm_session_id');
            this._refresh();
            return;
        }

        var farm_session_id = this.options['metadata']['farm_session_id'];

        me._refresh({'state': 'unknown'});
        
        /*
        $.ajax({
            url: '/rlplug_review/dailies/get_farm_session_status',
            data: {
                'job_id': farm_session_id
            },
            success: function(raw_result) {
                
                console.log(raw_result);
                var result = JSON.parse(raw_result);
                
                me._refresh(result);

                
                
            },
            error: function(serror) {
                $(me.selector_spinner).hide();
                
                console.log(serror);
            }
            
        });
        */
    }


    on_load_clicked(me, input_obj) {
        console.log('load clicked');

        $('#dailies-current').html('');

        var room_name = me.options['room_name'];
        var client_name = GLOBALS['username'];

        var kwargs = {
            'room_name': room_name,
            'sync': true
        };

        console.log('sending kwargs:');
        console.log(kwargs);

        window.rlp_controller.callFromClient(
            'load_room', [], kwargs);

        // me.spinner.message('Loading ' + room_name + ' ..');
    }


    on_delete_clicked(me, input_obj) {
        console.log('on delete clicked');

        var room_name = me.options['room_name'];
        var client_name = GLOBALS['username'];

        var kwargs = {
            'room_name': room_name
        };

        console.log('sending kwargs:');
        console.log(kwargs);
        
        $("#delete_dialog_room_name").html(room_name);
        $("#delete_dialog").dialog({
            modal: true,
            title: 'Delete Room "' + room_name + '"',
            buttons: [
                {
                    text: "Cancel",
                    click: function() {
                        $( this ).dialog( "close" );
                    }
                },
                {
                    text: "Delete Room",
                    click: function() {

                        window.rlp_controller.callFromClient(
                            'delete_room', [], kwargs);

                        $( this ).dialog( "close" );
                    }
                }
            ]
        });
    }
}


class RoomListView extends SSObject {

    constructor(selector, options) {
        super(selector, options);

        this.selector_toolbar = selector + '_toolbar';
        this.selector_room_list = selector + '_room_list';

    }


    init() {
        $(this.selector).html(
            '<div id="' + this.selector_toolbar.substring(1) + '"></div>' + 
            '<div style="height: 10px;"></div>' +
            '<div id="' + this.selector_room_list.substring(1) + '"></div>'
        );
        
        this.new_button = new Button(
            this.selector_toolbar,
            {
                'name': 'create',
                'label': 'New Room'
            }
        );
        this.new_button.connect('clicked', this, this.on_new_button_clicked);
        this.new_button.init();

        

        this.refresh_button = new Button(
            this.selector_toolbar,
            {
                'name': 'refresh',
                'label': 'Refresh'
            }
        );

        this.refresh_button.connect('clicked', this, this.on_refresh_clicked);
        this.refresh_button.init();



        // this.spinner = new Spinner(this.selector);
        // this.spinner.init();
    }


    refresh() {
        console.log('RoomListView refresh()')
        window.rlp_controller.callFromClient("get_rooms", [], {}, function(){});
    }


    create_new_room(input_obj) {
        console.log('RoomListView - create new room');
        console.log(input_obj);

        window.rlp_controller.callFromClient('create_room', [], input_obj);
        
    }


    on_refresh_clicked(me, input_obj) {

        console.log('RoomListView on_refresh_clicked()');
        me.refresh();
    }


    on_get_rooms(input_obj) {

        var me = this;

        $(this.selector_room_list).html('');

        console.log('RoomListView - on_get_rooms()');
        console.log(input_obj);

        jQuery.each(input_obj['result'], function(idx, entry) {

            var room_obj = new RoomListWidget(me.selector_room_list, entry);
            room_obj.init();
            room_obj.refresh();

            $(me.selector_room_list).append('<div style="height: 10px;"></div>');

        });
    }


    on_room_joined(input_obj) {
        window.spinner.hide();
        console.log('on room joined');

        console.log(input_obj);

        var room_name = input_obj['room_name'];

        
    }


    on_new_button_clicked(me, input_obj) {

        console.log('on new button clicked');

        $("#new_dialog").dialog({
            title: 'Create New Room',
            width: 400,
            modal: true,
            buttons: [
                {
                    text: "Cancel",
                    click: function() {
                        $( this ).dialog( "close" );
                    }
                },
                {
                    text: "Create New Room",
                    click: function() {

                        

                        var contents_empty = $('#new-dialog-media-1').prop('checked');
                        var contents_session = $('#new-dialog-media-2').prop('checked');
                        
                        var contents = '';
                        if (contents_empty) {
                            contents = 'empty';
                        } else if (contents_session) {
                            contents= 'current_session';
                        }

                        var kwargs = {
                            'room_name': $("#new_room_name").val(),
                            'contents': contents
                        }
                        console.log(contents_empty);
                        console.log(contents_session);

                        me.create_new_room(kwargs);
                        $( this ).dialog( "close" );
                    }
                }
            ]
        });

        
    }


    on_message_received(me, input_obj) {

        console.log('RoomListView ON MESSAGE RECEIVED');
        console.log(input_obj);

        if (input_obj.method == 'on_get_rooms') {
            me.on_get_rooms(input_obj.kwargs);

        } else if (input_obj.method == 'on_create_room') {
            me.refresh();

        } else if (input_obj.method == 'on_delete_room') {
            me.refresh();

        }
    }
}