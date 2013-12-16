<?php
if($_POST){
   //echo "recibo algo POST";
   
   //recibo los datos y los decodifico con PHP
   $tipo_procesamiento= $_POST["id_proces"];
   $tipo_procesami= $_POST["id_red"];
   
   echo $tipo_procesamiento;
   echo $tipo_procesami;
}else{
   echo "No recibí datos por POST";
}
?>