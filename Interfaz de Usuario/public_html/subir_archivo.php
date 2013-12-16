<?php 


if(isset($_FILES['archivo'])){ 
    $archivo = $_FILES['archivo'];
    //test de ejecucion de funciones
     
    //exec('aplicacion/algoritmos_serie/Egrises.c',$archivo);
    
    if(move_uploaded_file($archivo['tmp_name'], "imagenes_generadas/{$archivo['name']}")){
       echo 1;
    }else{
       echo 0;
    } 
 } 
?>