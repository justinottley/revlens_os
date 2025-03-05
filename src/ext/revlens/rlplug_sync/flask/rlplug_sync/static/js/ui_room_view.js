
class RoomViewWidget extends SSObject {

    constructor(selector, options) {
        super(selector, options);

        this.selector_accordion = selector + '_accordion';
        this.selector_current = selector + '_room_current';
        this.selector_current_title = selector + '_room_title';
        this.selector_list = selector + '_room_list';

        this.register_signal('message');
    }


    init() {
        $(this.selector).html('');
        $(this.selector).html(
            '<div id="' + this.selector_accordion.substring(1) + '" style="height: 100%;">' +
            '<h3><span class="room-current-title">-</span></h3><div id="' + this.selector_current.substring(1) + '"></div>' +
            '<h3>All Rooms</h3><div id="' + this.selector_list.substring(1) + '"></div>'
        );
        
        $(this.selector_list).css({
            'height': '100%'
        });

        this.current_room = new RoomWidget(this.selector_current, {'view': this});
        this.room_list = new RoomListView(this.selector_list, {'view': this});

        this.current_room.connect('leave_room', this, this.on_leave_room);
        this.connect('message', this.current_room, this.current_room.on_message_received);
        this.connect('message', this.room_list, this.room_list.on_message_received);
        
        $(this.selector_accordion).accordion(
            {
                'heightStyle': 'fill',
                'animate': 100,
                'active': 0, // room list as active,
                'activate': function(event, ui) {
                    console.log('accordion activated');
                    // console.log(ui);

                    $(ui.newPanel).css({
                        'height': ''
                    });

                }
            });
        
        // $(this.selector_accordion).accordion("option","active", 0);
        $(this.selector_accordion).accordion("option","active", 1);
        $(this.selector_accordion).accordion('refresh');

        this.current_room.init();
        this.room_list.init();

    }

    
    set_current_room_title(room_name) {

        var current_header = $(this.selector_accordion).accordion('option', 'header');
        var header_el = $(this.selector_current_tile + ' .accordion-header');

        $(".room-current-title").text(room_name);


    }


    set_properties(props) {

        
    }


    on_leave_room(me, input_obj) {
        me.set_current_room_title('-');
    }


    on_message_received(input_obj) {

        console.log('RoomViewWidge on_message_received()');
        console.log(input_obj);

        var method = input_obj.method;
        if (method == 'on_get_properties') {

            var props = input_obj.kwargs.properties;

            if (props['room_name'] != '') {
                this.set_current_room_title(props['room_name']);
                this.current_room.update(props['room_name']);
            }

        } else if (method == 'on_room_loaded') {
            this.set_current_room_title(input_obj.kwargs.room_name);
        }

        this.emit_signal('message', input_obj);

        /*
        if (input_obj.method == 'on_room_loaded') {
            this.current_room.
        }

        else {
            
        }
        */
    }
}