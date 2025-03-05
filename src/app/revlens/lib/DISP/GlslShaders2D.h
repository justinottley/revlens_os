//
//  Copyright (c) 2014-2019 Justin Ottley. All rights reserved.
//

static const char* vertex_shader_2D = R"(
        #version 120
        
        varying highp vec4 texc;
        attribute highp vec4 texCoord;
        attribute highp vec3 vertex;
        
        uniform vec2 viewport;
        uniform vec2 image;
        
        uniform mediump mat4 matrix;
        
        uniform float zoom;
        uniform vec2 pan;
        uniform vec2 layout_scale;
        uniform vec2 layout_pan;
    
        uniform int chan_order_rgb;
        uniform float videoGamma;

        mat4 translate(mat4 m, vec3 v) {
            mat4 Result = mat4(m);
            Result[3] = m[0] * v[0] + m[1] * v[1] + m[2] * v[2] + m[3];
            return Result;
        }

        mat4 scale(mat4 m, vec3 v) {
            mat4 Result = mat4(m);
            Result[0] = m[0] * v[0];
            Result[1] = m[1] * v[1];
            Result[2] = m[2] * v[2];
            Result[3] = m[3];
            return Result;
        }

        void main(void)
        {
            
            // vec2 translating = vec2(0.0, 0.0);
            
            // set initial scale to the aspect of the image - prevents
            // the image from filling the whole screen and instead be
            // displayed in the original aspect ratio
            vec2 scaling = vec2(image.x / viewport.x, image.y / viewport.y);

            scaling = scaling * vec2(layout_scale.x, layout_scale.y);
            
            // apply viewing scale - scale image down or up from actual size
            scaling = scaling * vec2(zoom);
            
            mat4 world = mat4(1); // matrix;
            
            world = translate(world, vec3(layout_pan.x, layout_pan.y, 0));

            world = translate(world, vec3(pan, 0));
            
            world = scale(world, vec3(scaling, 1));



            // projection setup elsewhere
            mat4 proj = mat4(1);

            mat4 worldViewProj = proj * world;

            gl_Position = worldViewProj * vec4(vertex, 1.0);
            texc = texCoord;
        })";


static const char* fragment_shader_2D = R"(
        #version 120

        uniform sampler2D texture;
        varying mediump vec4 texc;
        uniform int chan_order;
        uniform int colspace_linear;

        uniform int do_grayscale;
        uniform float greyscale_mix;

        uniform float alpha;
        uniform float opacity;
        uniform float videoGamma;

        vec3 greyscale(vec3 color, float alphaval) {
            float g = dot(color, vec3(0.299, 0.587, 0.114));
            return mix(color, vec3(g), alphaval);
        }

        vec4 greyscale(vec4 color, float gmix) {
            vec3 cresult = greyscale(color.rgb, gmix);
            vec4 result = vec4(cresult.rgb.r, cresult.rgb.g, cresult.rgb.b, color.a);
            return result;
        }

        void main()
        {
            int corder = 1;
            highp vec4 color;

            if (chan_order == 0)
            {
                color = vec4(texture2D(texture, texc.st).bgr, alpha); // , texture2D(texture, texc.st).a);
            }
            else if (chan_order == 1)
            {
                color = vec4(texture2D(texture, texc.st).rgba);
            }
            else if (chan_order == 2)
            {
                color = vec4(texture2D(texture, texc.st).rgb, alpha);
            }

            color = vec4(opacity, opacity, opacity, opacity) * color;

            // if (do_grayscale == 1)
            // {
            //     color = greyscale(vec3(color.r, color.g, color.b), greyscale_mix);
            // }

            if (colspace_linear == 1)
            {
                // FIXME: proper LUTs
                vec4 gammaVec = vec4(videoGamma, videoGamma, videoGamma, 1.0);
                color = pow(color, gammaVec);
            }
            
            // gl_FragColor = vec4(color.rgb, texture2D(texture, texc.st).a);
            // gl_FragColor = texture2D(texture, texc.st);
            gl_FragColor = color;
        })";
        