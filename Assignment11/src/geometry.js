var terrain_vertices = [
    -50,0,-50,
    -50,0,50,
    50,0,50,
    -50,0,-50,
    50,0,50,
    50,0,-50
];
var terrain_normals = [
    0,1,0,
    0,1,0,
    0,1,0,
    0,1,0,
    0,1,0,
    0,1,0,
];


//---------------------------
// definition of the cloth geometry
//---------------------------


var cloth_vertices = [];
var cloth_normals = [];
var cloth_triangles = [];
var cloth_size = 10;  // Number of rows/columns of vertices.

function create_cloth(){
    let step = 1.0/(cloth_size-1);
    // creating vertices
    for(let i = 0; i < cloth_size; i++){
        for(let j = 0; j < cloth_size; j++){
            cloth_vertices.push(j*step - 0.5 , 0.5 - i*step ,0);
            cloth_normals.push(0.0, 0.0, 1.0);
        }
    }
    // adding traingles
    for(let i = 0; i < cloth_size-1; i++){
        for(let j = 0; j < cloth_size-1; j++){
            let top = i * cloth_size + j;
            cloth_triangles.push(top, top + cloth_size, top + cloth_size + 1);
            cloth_triangles.push(top, top + cloth_size + 1, top + 1);
        }
    }
}

create_cloth();
