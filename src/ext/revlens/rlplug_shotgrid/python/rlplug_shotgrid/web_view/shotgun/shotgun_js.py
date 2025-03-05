#
# Copyright 2024 Justin Ottley
#
# Licensed under the terms set forth in the LICENSE.txt file
#

SHOTGUN_JS_INJECTION_SCRIPT = '''

// var d = document.createElement("div");
// d.innerHTML = "Lens Enabled";

// document.body.appendChild(d);

add_revlens_action_menu_items = function() {
    
    // Use the "new entity" button to determine what type of entity the view is displaying
    //
    var new_button_result = document.querySelectorAll('[sg_selector="button:new_entity"]');
    var entity_type = new_button_result[new_button_result.length - 1].firstChild.innerText;
    entity_type = entity_type.substring(4, entity_type.length);
    entity_type = entity_type.trim();
    
    console.log('Revlens - got entity type: ' + entity_type);
    
    if ((entity_type != "Version") &&
        (entity_type != "Playlist") &&
        (entity_type != "Sequence") &&
        (entity_type != "Review")) {
    
        console.log("Invalid page entity type for action menu item");
        return;
    }
    
    var action_menu_result = document.getElementsByClassName("action_menu");
    if (action_menu_result.length == 1) {
        var action_menu = action_menu_result[0];


        var scroll_area = action_menu.getElementsByClassName("sg_scroll_area")[0];
        // console.log(scroll_area);
        var rr = scroll_area.getElementsByClassName("revlens-load-menu-item");
        if (rr.length > 0)
        {
            return;
        }

        line_seps = action_menu.getElementsByTagName("hr");

        // console.log(line_seps);

        first_line_sep = line_seps[0];

        console.log(first_line_sep);

        revlens_play_el = document.createElement('div');
        revlens_play_el.classList.add('ui-button');
        revlens_play_el.style = "text-align: left; padding-left: 13%; width: 100%;";
        // revlens_play_el.classList.add('sg_menu_item');
        // revlens_play_el.classList.add('sg_menu_all_items');

        revlens_play_el.innerHTML = '<div class="revlens-load-menu-item sg_menu_icon sg_menu_item_checkmark" style=""><div class="sg_menu_icon_image_div blank" style="">&nbsp;</div></div><div class="sg_menu_icon " style="width:15px;"><div class="sg_menu_icon_image_div blank" style="max-width:15px;">&nbsp;</div></div><span class="sg_menu_item_content" title="">Load in Revlens</span>';
        revlens_play_el.onclick = function() {

            console.log("ON PLAY");

            var selected = document.getElementsByClassName("selected");
            var version_ids = [];
            for (var i=0; i != selected.length; ++i) {
            
                var sel_entry = selected[i];
                // console.log(sel_entry);
                record_id = sel_entry.attributes.getNamedItem("record_id");
                if (record_id != null)
                {
                    console.log("found record id:" + record_id);
                    version_ids.push(record_id.nodeValue);
                }
            }

            console.log(version_ids);

            action_menu.classList.add("sg_menu_hidden");

            // show_load_dialog(entity_type, version_ids);
            window.py.call('rlplug_shotgrid.cmds.request_load', [entity_type, version_ids]);

        }
        scroll_area.insertBefore(revlens_play_el, first_line_sep);

        // <div class="sg_menu_item sg_menu_all_items"><div class="sg_menu_icon sg_menu_item_checkmark" style=""><div class="sg_menu_icon_image_div blank" style="">&nbsp;</div></div><div class="sg_menu_icon " style="width:15px;"><div class="sg_menu_icon_image_div blank" style="max-width:15px;">&nbsp;</div></div><span sg_selector="menu:revlens_play" class="sg_menu_item_content" title="">Lens Play</span></div>
    }

}


_create_play_button = function() {

    var playb = document.createElement('button');
    playb.classList.add('ui-widget');
    playb.classList.add('ui-button');
    playb.classList.add('ui-corner-all');
    playb.classList.add('revlens-play-button');
    playb.setAttribute("style", "display: none;");
    
    playb.innerHTML = 'Play';
    
    return playb;
}


_create_load_button = function() {

    // Add button
    //
    var loadb = document.createElement('button');
    loadb.classList.add('ui-widget');
    loadb.classList.add('ui-button');
    loadb.classList.add('ui-corner-all');
    loadb.classList.add('revlens-load-button');
    
    loadb.setAttribute("style", "display: none;");
    loadb.innerHTML = 'Load';
    
    return loadb;
}



mutation_callback_thumbnail_buttons = function(mutation) {

    var main_result = document.getElementsByClassName("sg_main");
    if (main_result.length == 0) {
        return;
    }
    
    var target_node = main_result[0];
    
    if (mutation.type == 'attributes') {
    
        // console.log('The ' + mutation.attributeName + ' attribute was modified.');
        // console.log(mutationsList);
        // console.log(mutation);
        
        if ((mutation.attributeName == "class") && 
            (mutation.target.classList.contains("launch_movie_overlay"))) {
            
                // console.log(mutation);
                // console.log(mutation.target);

                mutation.target.classList.add("revlens_launch_movie_overlay");
                mutation.target.classList.remove("launch_movie_overlay");


                // Load Button
                //
                var loadb = _create_load_button();
                loadb.onclick = function() {
    
                    // console.log('mouseup');
                    // console.log(this.parentElement.parentElement.parentElement);
                    
                    var entity_type = this.parentElement.parentElement.parentElement.getAttribute("record_type");
                    var entity_id = this.parentElement.parentElement.parentElement.getAttribute("record_id");

                    console.log('LOAD REQUESTED');
                    console.log(entity_type);
                    console.log(entity_id);
                    // show_load_dialog(entity_type, [entity_id]);
                    window.py.call('rlplug_shotgun.cmds.request_load', [entity_type, entity_id]);

                }

                mutation.target.appendChild(loadb);
                
                mutation.target.parentElement.onmouseover = function() {
                    // console.log('mouseover');
                    // console.log(this);
                    //
                    // TODO FIXME: Switch to css/hover?
                    //
                    // NOTE: No play button, just load button
                    // pb = this.getElementsByClassName('revlens-play-button');
                    // if (pb.length == 1) {                                
                    //    pb[0].setAttribute("style", "display: inline-block; position: absolute; top: 50%; left: 40%; transform: translate(-50%, -50%); font-size: 1.0em;");
                    // }
                    
                    
                    loadb = this.getElementsByClassName('revlens-load-button');
                    if (loadb.length == 1) {
                        // loadb[0].setAttribute("style", "display: inline-block; position: relative; top: -30px; left: 40px; font-size: 1.0em;");
                        loadb[0].setAttribute("style", "display: inline-block; position: absolute; top: 50%; left: 50%; transform: translate(-50%, -50%); font-size: 1.0em;");
                    }
                    
                }
                
                mutation.target.parentElement.onmouseout = function() {
                
                    pb = this.getElementsByClassName('revlens-play-button');
                    if (pb.length == 1) {
                        pb[0].setAttribute("style", "display: none;");
                    }
                    
                    loadb = this.getElementsByClassName('revlens-load-button');
                    if (loadb.length == 1) {
                        loadb[0].setAttribute("style", "display: none;");
                    }
                    
                }
                mutation.target.parentElement.onmouseup = function() {
                    
                    
                }
                
                // console.log(mutation.target.parentElement);
        }
    }
}


mutation_callback_thumbnail_menu = function(mutation) {

    var main_result = document.getElementsByClassName("sg_main");
    if (main_result.length == 0) {
        return;
    }
    
    var target_node = main_result[0];
    
    if (mutation.type == 'attributes') {
    
        // console.log('The ' + mutation.attributeName + ' attribute was modified.');
        // console.log(mutationsList);
        // console.log(mutation);
        
        if ((mutation.attributeName == "class") && 
            (mutation.target.classList.contains("sg_menu"))) {

            }
    }
}

mutation_callback_action_menu_item = function(mutation) {

    if ((mutation.attributeName == "id") &&
        (mutation.target.parentElement.classList.contains("action_menu"))) {

        // console.log('"More" button detected');
        // console.log(mutation);
        // console.log(mutation.target);

        add_revlens_action_menu_items();
    }
}


mutation_callback_media_page_more_menu = function(mutation) {
    if (mutation.target.classList.contains("sg_menu_body") &&
        mutation.target.classList.contains("sg_scroll_area")) {

        var menu_parent = mutation.target;

        rr = menu_parent.getElementsByClassName("revlens-load-menu-item");
        if (rr.length != 0)
        {
            return;
        }

        // console.log('sg_menu detected');
        // console.log(mutation);
        // console.log(mutation.target);
        // console.log('first element:');

        line_seps = menu_parent.getElementsByTagName("hr");
        if (line_seps.length < 3) {
            return;
        }
        first_line_sep_el = line_seps[0];

        // console.log('line seps:');
        // console.log(line_seps);


        // add_revlens_action_menu_items();
        revlens_play_el = document.createElement('div');
        revlens_play_el.classList.add('ui-button');
        revlens_play_el.classList.add('revlens-load-menu-item');
        revlens_play_el.style = "text-align: left; padding-left: 13%; width: 100%;";
        // revlens_play_el.classList.add('sg_menu_item');
        // revlens_play_el.classList.add('sg_menu_all_items');
        
        revlens_play_el.innerHTML = '<div class="sg_menu_icon sg_menu_item_checkmark" style=""><div class="sg_menu_icon_image_div blank" style="">&nbsp;</div></div><div class="sg_menu_icon " style="width:15px;"><div class="sg_menu_icon_image_div blank" style="max-width:15px;">&nbsp;</div></div><span class="sg_menu_item_content" title="">Load in Revlens</span>';
        
        revlens_play_el.onclick = function() {
            
            console.log("LOAD CLICKED");
            
            var selected = document.getElementsByClassName("selected");
            var version_ids = [];
            for (var i=0; i != selected.length; ++i) {
            
                var sel_entry = selected[i];
                // console.log(sel_entry);
                record_id = sel_entry.attributes.getNamedItem("record_id");
                if (record_id != null)
                {
                    console.log("found record id:" + record_id);
                    version_ids.push(record_id.nodeValue);
                }
            }

            console.log(version_ids);

            menu_parent.classList.add("sg_menu_hidden");

            // show_load_dialog(entity_type, version_ids);
            window.py.call('rlplug_shotgrid.cmds.request_load', ['Version', version_ids]);

        }

        menu_parent.insertBefore(revlens_play_el, first_line_sep_el);

    }
}

mutation_callback_media_page = function(mutation) {
    var main_result = document.getElementsByClassName("sgw_media_center");
    if (main_result.length == 0) {
        return;
    }
    
    // console.log(mutation.target.className);

    if ((mutation.target.className == 'sg_reset_ul') &&
        (mutation.target.firstChild.classList.contains("version"))) {

        // console.log(mutation);
        var versions = mutation.target.getElementsByClassName('thumb');
        // console.log(versions.length);
        
        jQuery.each(versions, function(idx, thumbnail_el) {
            
            if (thumbnail_el.getAttribute('sg_selector') != 'cell_content:image') {
                return;
            }

            // console.log(thumbnail_el);

            // Load button
            //
            var loadb = _create_load_button();
            loadb.onclick = function() {
                
                var version_src = thumbnail_el.getElementsByTagName('img')[0].getAttribute('src');
                var version_src_parts = version_src.split('/');
                var version_id = version_src_parts[version_src_parts.length - 1];

                console.log(version_id);

                // show_load_dialog('Version', [version_id]);
                window.py.call('rlplug_shotgrid.cmds.request_load', ['Version', version_id]);

            }

            thumbnail_el.appendChild(loadb);
            
            thumbnail_el.onmouseover = function() {
                // console.log('mouseover');
                // console.log(this);
                //
                // TODO FIXME: Switch to css/hover?
                //
                // NOTE: no play button, just load button
                //
                // pb = this.getElementsByClassName('revlens-play-button');
                // if (pb.length == 1) {                                
                //     pb[0].setAttribute("style", "display: inline-block; position: absolute; top: 50%; left: 40%; transform: translate(-50%, -50%); font-size: 1.0em;");
                // }
                
                
                loadb = this.getElementsByClassName('revlens-load-button');
                if (loadb.length == 1) {
                    // loadb[0].setAttribute("style", "display: inline-block; position: relative; top: -30px; left: 40px; font-size: 1.0em;");
                    loadb[0].setAttribute("style", "display: inline-block; position: absolute; top: 50%; left: 50%; transform: translate(-50%, -50%); font-size: 1.0em;");
                }
                
            }
            
            thumbnail_el.onmouseout = function() {
            
                pb = this.getElementsByClassName('revlens-play-button');
                if (pb.length == 1) {
                    pb[0].setAttribute("style", "display: none;");
                }
                
                loadb = this.getElementsByClassName('revlens-load-button');
                if (loadb.length == 1) {
                    loadb[0].setAttribute("style", "display: none;");
                }
                
            }
        });
    }
}



show_load_dialog = function(entity_type, entity_ids) {

    window.revlens.pycall('get_track_names', [], function(result) {

        // console.log('RETURN FROM GET TRACKS??');
        // console.log(result);

        jQuery('#revlens_load_dialog_text')
            .html(entity_ids.length + ' ' + entity_type + 's');

        var track_menu_markup = '<select id="revlens_load_dialog_track_select">';
        jQuery.each(result, function(idx, value) {
            track_menu_markup += '<option value="' + idx + '">' + value + '</option>';
        });
        track_menu_markup += '</select>';

        jQuery('#revlens_load_dialog_tracks')
            .html(track_menu_markup);

        // Dont paint selectmenu, doesn't display all that well
        //
        // jQuery('#revlens_load_dialog_track_select').selectmenu();

        jQuery("#revlens_load_dialog").dialog({
            'title': 'Revlens - Load ' + entity_type,
            'buttons': {
                'Cancel': function() {
                    jQuery(this).dialog('close');
                },
                'Load': function() {
                    
                    var track_select_el = document.getElementById('revlens_load_dialog_track_select');
                    var selected_track_idx = track_select_el.options[track_select_el.selectedIndex].value;

                    console.log(selected_track_idx);

                    var pos_select_el = document.getElementById('revlens_load_dialog_position');
                    var selected_position = pos_select_el.options[pos_select_el.selectedIndex].text;

                    console.log(selected_position);

                    window.revlens.pycall("load_entity", [entity_type, entity_ids, selected_track_idx, selected_position]);
            
                    jQuery(this).dialog('close');
                }
            }
        });

        

    });

}


setup_load_dialog = function() {

    var load_dialog_div = document.createElement('div');
    var label_div = document.createElement('div');

    var load_markup = '';
    load_markup += '<div>';
    load_markup += '<div id="revlens_load_dialog_text"></div><p></p>';
    load_markup += '<div><span style="float: left; width: 80px;">Track:</span><span id="revlens_load_dialog_tracks"></span></div><p></p>';
    load_markup += '<div><span style="float: left; width: 80px;">Position:</span>';
    load_markup += '  <select id="revlens_load_dialog_position">';
    load_markup += '    <option>Start</option>';
    load_markup += '    <option>End</option>';
    load_markup += '    <option>Current Frame</option>';
    load_markup += '  </select>';
    load_markup += '</div>';
    load_markup += '</div>';

    label_div.innerHTML = load_markup;

    load_dialog_div.appendChild(label_div);

    load_dialog_div.id = 'revlens_load_dialog';
    document.body.appendChild(load_dialog_div);
    
    // Dont paint selectmenu, doesn't display all that well
    //
    // jQuery('#revlens_load_dialog_position').selectmenu();

    jQuery('#revlens_load_dialog').hide();
}



body_callback = function(mutationsList) {

    // Callback function to execute when mutations are observed

    // console.log('MUTATION');

    for(var mutation of mutationsList) {
        // mutation_callback_thumbnail_buttons(mutation);
        mutation_callback_action_menu_item(mutation);
        mutation_callback_media_page_more_menu(mutation);
        // mutation_callback_media_page(mutation);
    }

    // Hide shotgun header
    //
    jQuery('.header_content').css({'min-height':0});
    jQuery('.header_content .stream_detail_header .header_table').hide();
}


// Options for the observer (which mutations to observe)
var config = { attributes: true, childList: true, subtree: true };
    
// Create an observer instance linked to the callback function
var observer = new MutationObserver(body_callback);

// Start observing the target node for configured mutations
observer.observe(document.body, config);

console.log('Revlens - Mutation Obvserver running - observing');



console.log('starting qt webchannel');
try {
    new QWebChannel(qt.webChannelTransport, function(channel) {
        
        console.log('Revlens - setting up Qt WebChannel for python');
        window.py = channel.objects.py;
        console.log('Revlens - WebChannel set successfully');
        
    });
} catch(err) {

    console.log('Revlens - Error starting qt webchannel');
    console.log(err);
    console.log('Revlens - initializing revlens standin');

    window.revlens = {
        
        pycall: function(method, arg_list) {
            console.log('revlens.pycall() [STANDIN] ' + method);
        }
    }
}

setup_load_dialog();

'''



SHOTGUN_JS_INJECTION_SCRIPT_TEST = '''

console.log('HELLO WORLD');


'''