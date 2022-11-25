function getScalingMat(vec3Scale=vec3.fromValues(1,1,1), baseMat=mat4.create()){
    let resMat = mat4.create()
    mat4.scale(resMat, baseMat, vec3Scale);
    return resMat;
}

function getTranslationMat(vec3Translation=vec3.fromValues(0,0,0), baseMat=mat4.create()){
    let resMat = mat4.create()
    mat4.translate(resMat, baseMat, vec3Translation);
    return resMat;
}

function getRotationMat(vec3Rotation=vec3.fromValues(0,0,0), baseMat=mat4.create()){
    let resMat = mat4.create();
    let resMatx = mat4.create();
    let resMaty = mat4.create();
    let resMatz = mat4.create();
    
    mat4.fromRotation(resMatx, vec3Rotation[0], vec3.fromValues(1,0,0));
    mat4.fromRotation(resMaty, vec3Rotation[1], vec3.fromValues(0,1,0));
    mat4.fromRotation(resMatz, vec3Rotation[2], vec3.fromValues(0,0,1));

    mat4.multiply(resMat, resMaty, resMatz);
    mat4.multiply(resMat, resMat, resMatx);

    return resMat;
}

function getTransformationMat(vec3Translation, vec3Rotation, vec3Scale){
    return getScalingMat(vec3Scale,getRotationMat(vec3Rotation,getTranslationMat(vec3Translation)));
}


function setTransformationUniforms(vec3Translation, vec3Rotation, vec3Scale) {

    const translateMat = getTranslationMat(vec3Translation);
    const rotationMat = getRotationMat(vec3Rotation);
    const scaleMat = getScalingMat(vec3Scale);

    let translationlMatLocation = gl.getUniformLocation(shaderProgram,"translationMatrix");
    gl.uniformMatrix4fv(translationlMatLocation, false, translateMat);

    let rotationMatLocation = gl.getUniformLocation(shaderProgram,"rotationMatrix");
    gl.uniformMatrix4fv(rotationMatLocation, false, rotationMat);

    let scaleMatLocation = gl.getUniformLocation(shaderProgram,"scaleMatrix");
    gl.uniformMatrix4fv(scaleMatLocation, false, scaleMat);
}