#
# Copyright 2024 Justin Ottley
#
# Licensed under the terms set forth in the LICENSE.txt file
#

CSS_TEMPLATE = '''
css = document.createElement('style');
css.type = 'text/css';
css.rel = 'stylesheet';
document.head.appendChild(css);
css.innerText = '{css_contents}';
'''


CSS_FILE_TEMPLATE = '''
css = document.createElement('link');
css.type = 'text/css';
css.rel = 'stylesheet';
css.href = 'file:///tmp/test.css';
document.head.appendChild(css);
'''