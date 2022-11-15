var gl; // WebGL context
var shaderProgram; // the GLSL program we will use for rendering
var triangle_vao; // the vertex array object for the triangle
var cube_vao; // the vertex array object for the cube

// The function to initialize the WebGL canvas
function initWebGL() {
    var canvas = document.getElementById("webgl-canvas");
    gl = canvas.getContext("webgl2");

    // Keep the size of the canvas for later rendering
    gl.viewportWidth = canvas.width;
    gl.viewportHeight = canvas.height;

    // Check for errors
    if (gl) {
        console.log("WebGL succesfully initialized.");
    } else {
        console.log("Failed to initialize WebGL.")
    }
}

// This function compiles a shader
function compileShader(shader, source, type, name = "") {
    // link the source of the shader to the shader object
    gl.shaderSource(shader, source);
    // compile the shader
    gl.compileShader(shader);
    // check for success and errors
    if (gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
        console.log(name + " shader compiled succesfully.");
    } else {
        console.log(name + " vertex shader error.")
        console.log(gl.getShaderInfoLog(shader));
    }
}

// This function links the GLSL program by combining different shaders
function linkProgram(program, vertShader, fragShader) {
    // attach vertex shader to the program
    gl.attachShader(program, vertShader);
    // attach fragment shader to the program
    gl.attachShader(program, fragShader);
    // link the program
    gl.linkProgram(program);
    // check for success and errors
    if (gl.getProgramParameter(program, gl.LINK_STATUS)) {
        console.log("The shaders are initialized.");
    } else {
        console.log("Could not initialize shaders.");
    }
}

function createGLSLPrograms() {
    const vertexShaderCode =
        `#version 300 es
                in vec3 a_position;
                in vec3 a_color;
                out vec3 v_color;

                uniform mat4 rotationMatrix;
                void main(){
                    gl_Position = rotationMatrix * vec4(a_position,1.0); // extra code for interactive rotation, it does need to be modified
                    v_color = a_color;
                }`;
    const fragmentShaderCode =
        `#version 300 es
                precision mediump float;

                in vec3 v_color;
                out vec4 out_color;
                void main(){
                    out_color = vec4(v_color,1.0);
                    // out_color = vec4(0.0, 0.61, 0.87, 1.0);
                }`;

    var vertexShader = gl.createShader(gl.VERTEX_SHADER);
    compileShader(vertexShader, vertexShaderCode, gl.VERTEX_SHADER, "Vertex shader");
    // Creating fragment shader
    var fragmentShader = gl.createShader(gl.FRAGMENT_SHADER);
    compileShader(fragmentShader, fragmentShaderCode, gl.VERTEX_SHADER, "Fragment shader");
    // Creating and linking the program
    shaderProgram = gl.createProgram();
    linkProgram(shaderProgram, vertexShader, fragmentShader);

    shaderProgram.rotationMatrix = gl.getUniformLocation(shaderProgram, "rotationMatrix"); // extra code for interactive rotation, it does need to be modified
}

function initBuffers(vertices, colors) {
    //----------------------------------------------------------------------------
    // First we need to create buffers on the GPU and copy there our data
    //----------------------------------------------------------------------------
    // create a buffer for vertices
    var triangleVertexBuffer = gl.createBuffer();
    // bind the buffer
    gl.bindBuffer(gl.ARRAY_BUFFER, triangleVertexBuffer);
    // copy the data from the CPU to the buffer (GPU)
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(vertices), gl.STATIC_DRAW);


    // Buffers for colors
    var triangleColorBuffer = gl.createBuffer();
    // bind the buffer
    gl.bindBuffer(gl.ARRAY_BUFFER, triangleColorBuffer);
    // copy the data from the CPU to the buffer (GPU)
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array(colors), gl.STATIC_DRAW);

    //----------------------------------------------------------------------------
    // At this point our vertices are already on the GPU.
    // We have to specify how the data will flow through the graphics pipeline.
    // We do it by setting up the vertex array objects which store information about buffers and how the connect to attributes.
    //----------------------------------------------------------------------------

    // create a vertex array object (VAO) to store information about buffers and attributes
    vao = gl.createVertexArray();
    // bind the VAO
    gl.bindVertexArray(vao);

    //----------------------------------------------------------------------------
    // Now we need to set up all the buffers and attributes for rendering
    //----------------------------------------------------------------------------
    // bind the buffer with vertices
    gl.bindBuffer(gl.ARRAY_BUFFER, triangleVertexBuffer);
    // get position of the attribute in the vertex shader
    var positionAttributeLocation = gl.getAttribLocation(shaderProgram, "a_position");
    // enable attribute for the positions
    gl.enableVertexAttribArray(positionAttributeLocation);
    // bining the vertex buffer with positions to the correct attribute
    gl.vertexAttribPointer(positionAttributeLocation, 3, gl.FLOAT, false, 0, 0);


    // Configure the attributes for color
    // bind the color buffer
    gl.bindBuffer(gl.ARRAY_BUFFER, triangleColorBuffer);
    // get the color attribute location
    var colorAttributeLocation = gl.getAttribLocation(shaderProgram, "a_color");
    // enable the color attribute
    gl.enableVertexAttribArray(colorAttributeLocation);
    // bind the buffer to the attribute
    gl.vertexAttribPointer(colorAttributeLocation, 3, gl.FLOAT, false, 0, 0);

    return vao;
}

function draw() {
    // extra code for interactive rotation, it does need to be modified
    var rotation = document.getElementById("rotation");
    var rotationMatrix = mat4.create();
    mat4.fromRotation(rotationMatrix, -(rotation.value - 100) / 100 * Math.PI, vec3.fromValues(-0.2, 1, 0));
    //------------------------------------------------------------------

    // set the size of our rendering area
    gl.viewport(0, 0, gl.viewportWidth, gl.viewportHeight);

    // setting the background color
    gl.clearColor(0.2, 0.2, 0.2, 1.0);
    // clear the rendering area
    gl.clear(gl.COLOR_BUFFER_BIT);

    // Enable face culling and depth test for the 2nd part of the assignment
    gl.enable(gl.CULL_FACE);
    gl.enable(gl.DEPTH_TEST);

    //----------------------------------------------------------------------------
    // Now we are ready to render
    //----------------------------------------------------------------------------
    // enable the GLSL program for the rendering
    gl.useProgram(shaderProgram);
    gl.uniformMatrix4fv(shaderProgram.rotationMatrix, false, rotationMatrix); // extra code for interactive rotation, it does need to be modified
    // bind the VAO (this restores the state from when we were creating the VAO)

    // Check
    let vao = triangle_vao;
    let n_triangles = 3;

    switch(document.querySelector("#mesh").value) {
        case "cube":
            vao = cube_vao;
            n_triangles = 2*3*6;
            break;
        default:
            vao = triangle_vao;
            n_triangles = 3;
            break;
    }

    gl.bindVertexArray(vao);
    // draw all the triangles
    gl.drawArrays(gl.TRIANGLES, 0, n_triangles); // 3 for triangle, 3*2*6 for a cube

    window.requestAnimationFrame(function () { draw(); });
}
function start() {
    const dcube = cube(0, 0, 0, 1);
    const dtriangle = triangle(0, 0, 0, 1);

    // initialze WebGL
    initWebGL();
    // create GLSL programs
    createGLSLPrograms();
    // initialize all the buffers and set up the vertex array objects (VAO)
    triangle_vao = initBuffers(dtriangle.vertices, dtriangle.colors);
    cube_vao = initBuffers(dcube.vertices, dcube.colors);

    // draw
    draw();
}