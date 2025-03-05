//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

static const char* vertex_shader_3D =
    "attribute highp vec4 vertex;\n"
    "attribute mediump vec4 texCoord;\n"
    "varying mediump vec4 texc;\n"
    "uniform mediump mat4 matrix;\n"
    "void main(void)\n"
    "{\n"
    "    gl_Position = matrix * vertex;\n"
    "    texc = texCoord;\n"
    "}\n";

    
static const char *fragment_shader_3D =
    "uniform sampler2D texture;\n"
    "varying mediump vec4 texc;\n"
    "void main(void)\n"
    "{\n"
    "    gl_FragColor = texture2D(texture, texc.st);\n"
    "}\n";
