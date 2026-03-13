class Spinner extends SSObject {

    constructor(selector_parent, options) {
        var selector = selector_parent + '-spinner';
        super(selector, options);

        this.selector_parent = selector_parent;
        this.selector_message = selector + '-spinner-msg';
    }

    init() {

        $(this.selector_parent).append(
            '<div id="' + this.selector.substring(1) + '">' +
            '<img width="15" src="../../shared/static/images/ajax-loader.gif"/>' +
            '<span id="' + this.selector_message.substring(1) + '"></span>'
        );

        $(this.selector).hide();
    }


    message(msg) {
        $(this.selector).show();
        $(this.selector_message).html(msg);
    }

    hide() {
        $(this.selector).hide();
    }
}