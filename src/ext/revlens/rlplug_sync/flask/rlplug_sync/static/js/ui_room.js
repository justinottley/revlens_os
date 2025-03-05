class RoomWidget extends SSObject {

    constructor(selector, options) {
        // selector_parent = selector;
        // var selector = selector_parent 
        super(selector, options);

        this.room_name = null;
        this.register_signal('leave_room');
    }


    init() {

        console.log('RoomWidget init()');

        $(this.selector).html('');
        
        this.leave_button = new Button(
            this.selector,
            {'name': 'leave',
             'label': 'Leave'});
        
        this.leave_button.connect('clicked', this, this.on_leave_clicked);
        this.leave_button.init();
    }


    update(room_name) {
        console.log('RoomWidget update()');

        this.room_name = room_name;
        // var room_name = input_obj['room_name'];

        console.log(room_name);

        // $(this.selector).html('');
        
    }


    on_leave_clicked(me, input_obj) {

        console.log('RoomWidget on_leave_clicked() - ' + me.room_name);

        if (me.room_name != null) {
            window.rlp_controller.callFromClient('leave_room', [], {'room_name': me.room_name});
            me.room_name = null;
            me.emit_signal('leave_room', {});
        }
        
    }


    on_message_received(me, input_obj) {

        if (input_obj.method == 'on_room_loaded') {
            me.update(input_obj.kwargs.room_name);
        }
    }
}