<?php 
 if(isset($_FILES['archivo'])){ 
    $archivo = $_FILES['archivo']; 
    if(move_uploaded_file($archivo['tmp_name'], "archivos_subidos/{$archivo['name']}")){
       echo 1;
    }else{
       echo 0;
    } 
 } 
?>