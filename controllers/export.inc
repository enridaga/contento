<?php
require_once 'default.inc';
require_once 'taxonomies.inc';
class ExportController extends DefaultController {
	private $_enableSesson = TRUE;
	
	public function __construct($args = array()){
		if(isset($args['action_parameter']) && isset($args[$args['action_parameter']])){
			if($args[$args['action_parameter']] == 'doexport'){
				$this->_enableSesson = false;
			}
		}
		parent::__construct($args);
	}
	
	protected function _enableSession(){
		return $this->_enableSesson;
	}
	
	public function model() {
		$model = parent::model ();
		$model ['title'] = 'Export profiles';
		return $model;
	}
	private function _setDefault(&$arr, $key, $default) {
		if (! array_key_exists ( $key, $arr )) {
			$arr [$key] = null;
		}
		if (is_array ( $arr [$key] ) && empty ( $arr [$key] )) {
			$arr [$key] = $default;
		} elseif (is_array ( $arr [$key] ) && !empty ( $arr [$key] ) && is_array($default)) {
			foreach($default as $dk => $dv){
				if(!array_key_exists($dk, $arr[$key])){
					$arr[$key][$dk] = $dv;
				}
			}
		} elseif ($arr [$key] == '') {
			$arr [$key] = $default;
		}
	}
	private function _profile($p = array()) {
		$this->_setDefault ( $p, 'topclass', 'http://www.example.org/contento/Concept' );
		$this->_setDefault ( $p, 'subClassOf', 'http://www.w3.org/2000/01/rdf-schema#subClassOf');
		$this->_setDefault ( $p, 'class_to_attribute', 'http://www.example.org/contento/attribute' );
		$this->_setDefault ( $p, 'class_to_proper_attribute', 'http://www.example.org/contento/proper-attribute' );
		$this->_setDefault ( $p, 'class_to_object', 'http://www.example.org/contento/object' );
		$this->_setDefault ( $p, 'class_to_proper_object', 'http://www.example.org/contento/proper-object' );
		$this->_setDefault ( $p, 'object_to_class', 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type' );
		$this->_setDefault ( $p, 'proper_object_to_class', 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type' );
		$this->_setDefault ( $p, 'object_to_attribute', 'http://www.example.org/contento/has' );
		$this->_setDefault ( $p, 'object_to_proper_attribute', 'http://www.example.org/contento/has-proper' );
		$this->_setDefault ( $p, 'proper_object_to_attribute', 'http://www.example.org/contento/proper-has' );
		$this->_setDefault ( $p, 'proper_object_to_proper_attribute', 'http://www.example.org/contento/proper-has' );
		$this->_setDefault ( $p, 'namespace', 'http://www.example.org/contento/ns/' );
		$this->_setDefault ( $p, 'types', array (
				'attribute' => 'literal',
				'object' => 'uri' 
		) );
		$this->_setDefault ( $p, 'addns', array (
				'attribute' => 0,
				'object' => 0 
		) );
		$this->_setDefault ( $p, 'skip', array (
				'concept_label_empty' => 1,
				'class_to_attribute' => 0,
				'class_to_proper_attribute' => 0,
				'class_to_object' => 0,
				'class_to_proper_object' => 0,
				'object_to_class' => 0,
				'proper_object_to_class' => 0,
				'object_to_attribute' => 0,
				'object_to_proper_attribute' => 0,
				'proper_object_to_attribute' => 0,
				'proper_object_to_proper_attribute' => 0 
		) );
		
		return $p;
	}
	public function indexAction() {
		$this->listAction ();
	}
	public function newprofileAction() {
		$model = $this->model ();
		$model ['title'] = 'New export profile';
		$model ['page'] = 'export-profile';
		$model ['data'] = array (
				'profile' => $this->_profile () 
		);
		$v = new MVC_Html ( $model );
		$v->render ( 'layout' );
	}
	public function editprofileAction() {
		$db = DataAccess::instance ();
		$p = $db->profile ( $this->get ( 'profile' ), $this->_userid () );
		
		if (! $p ['writable']) {
			$this->_forbidden ( 'Cannot edit export profile' );
			die ();
		}
		$model = $this->model ();
		$model ['title'] = 'Edit export profile';
		$model ['page'] = 'export-profile';
		$model ['data'] = array (
				'id' => $p ['id'],
				'name' => $p ['name'],
				'description' => $p ['description'],
				'profile' => $this->_profile ( unserialize ( $p ['data'] ) ) 
		);
		$v = new MVC_Html ( $model );
		$v->render ( 'layout' );
	}
	public function listAction() {
		$d = DataAccess::instance ();
		$p = $d->listExportprofiles ( $this->_userid () );
		$model = $this->model ();
		$model ['title'] = 'Export profiles';
		$model ['page'] = 'exportprofiles';
		$model ['data'] = array (
				'profiles' => $p 
		);
		$v = new MVC_Html ( $model );
		$v->render ( 'layout' );
	}
	
	public function exportAction() {
		$db = DataAccess::instance ();
		$t = $db->listTaxonomies ();
		$profile = $this->_profile ();
		$model = $this->model ();
		$model ['title'] = 'Export Taxonomy as Ontology';
		$model ['page'] = 'export-taxonomy';
		$model ['data'] = array (
			'taxonomies' => $t,
			'profile' => $profile,
			'profiles' => $db->listExportprofiles () 
		);
		$v = new MVC_Html ( $model );
		$v->render ( 'layout' );
	}
	
	public function doexportAction(){
		$this->postexportAction();
	}
	
	public function doexport($taxonomy, $profile) {
		$db = DataAccess::instance ();
		
		$ns = $profile ['namespace'];
		$types = $profile ['types'];
		$addns = $profile ['addns'];
		$skip = $profile ['skip'];

		ob_start ();
		header ( "Content-Type: text/plain; charset=UTF-8" );
		// List concepts
		$concepts = $db->concepts ( $taxonomy );
		$spo = create_function ( '$s,$p,$o', 'print "<$s> <$p> <$o> . " . PHP_EOL ; ' );
		$spl = create_function ( '$s,$p,$o', '$o = str_replace(\'"\',\'\\"\',$o); print "<$s> <$p> \"\"\"$o\"\"\" . " . PHP_EOL ; ' );
		$sptl = create_function ( '$s,$p,$o,$t', '$o = str_replace(\'"\',\'\\"\',$o); print "<$s> <$p> \"\"\"$o\"\"\"^^<$t> . " . PHP_EOL ; ' );
		$spll = create_function ( '$s,$p,$o,$l', '$o = str_replace(\'"\',\'\\"\',$o); print "<$s> <$p> \"\"\"$o\"\"\"@$l . " . PHP_EOL ; ' );
		foreach ( $concepts as $c ) {
			if($skip['concept_label_empty'] && $c['name'] == ''){
				continue;
			}
			// 
			// add proper intent/extents
			$c['top'] = $db->taxonomyParentsData($c['id']);
			$c['bottom'] = $db->taxonomyChildrenData($c['id']);
			TaxonomiesController::specifyFeatures ( $c );

			$s = $ns . $c ['id'];
			// rdf:type
			$spo ( $s, 'http://www.w3.org/1999/02/22-rdf-syntax-ns#type', $profile ['topclass'] );
			$spl ( $s, 'http://www.w3.org/2000/01/rdf-schema#label', $c ['name'] );
			$spl ( $s, 'http://www.w3.org/2000/01/rdf-schema#comment', $c ['comment'] );
			foreach ( $c ['top'] as $b ) {
				$spo ( $s, $profile ['subClassOf'], $ns . $b['id'] );
			}
			
			// Intent
			$intent = explode ( ',', $c ['intent'] );
			$extent = explode ( ',', $c ['extent'] );
			foreach ( $intent as $in ) {
				if ($in == '')
					continue;
					// class to attribute
				if (! $skip ['class_to_attribute']) {
					if ($types ['attribute'] == 'uri') {
						$in = ($addns ['attribute']) ? $ns . $in : $in;
						$spo ( $s, $profile ['class_to_attribute'], $in );
					} else {
						$spl ( $s, $profile ['class_to_attribute'], $in );
					}
				}
				// class to proper attribute
				if (in_array ( $in, $c ['properIntent'] )) :
					if (! $skip ['class_to_proper_attribute']) {
						if ($types ['attribute'] == 'uri') {
							$in = ($addns ['attribute']) ? $ns . $in : $in;
							$spo ( $s, $profile ['class_to_proper_attribute'], $in );
						} else {
							$spl ( $s, $profile ['class_to_proper_attribute'], $in );
						}
					}
					
				
				endif;
			}
			// Extent
			foreach ( $extent as $ex ) {
				if ($ex == '')
					continue;
					// class to object
				if (! $skip ['class_to_object']) {
					if ($types ['object'] == 'uri') {
						$spo ( $s, $profile ['class_to_object'], ($addns ['object']) ? $ns . $ex : $ex );
					} else {
						$spl ( $s, $profile ['class_to_object'], $ex );
					}
				}
				// class to proper object
				if (in_array ( $ex, $c ['properExtent'] )) {
					if (! $skip ['class_to_proper_attribute']) {
						if ($types ['attribute'] == 'uri') {
							$spo ( $s, $profile ['class_to_proper_attribute'],  ($addns ['attribute']) ? $ns . $ex : $ex );
						} else {
							$spl ( $s, $profile ['class_to_proper_attribute'], $ex );
						}
					}
				}
				
				// If Object is a Resource
				if ($types ['object'] == 'uri') {
					$ex = ($addns ['object'] ? $ns . $ex : $ex);
					// object to class
					if (! $skip ['object_to_class']) {
						$spo ( $ex, $profile ['object_to_class'], $s );
					}
					
					foreach ( $intent as $in ) {
							
						// object to attribute
						if (! $skip ['object_to_attribute']) {
							if ($types ['attribute'] == 'uri') {
								$in = ($addns ['attribute'] ? $ns . $in : $in);
								$spo ( $ex, $profile ['object_to_attribute'], $in );
							} else {
								$spl ( $ex, $profile ['object_to_attribute'], $in );
							}
						}
						
						// object to proper attribute
						if (in_array ( $in, $c ['properIntent'] )) {
							if (! $skip ['object_to_proper_attribute']) {
								if ($types ['attribute'] == 'uri') {
									$in = ($addns ['attribute'] ? $ns . $in : $in);
									$spo ( $ex, $profile ['object_to_proper_attribute'], $in );
								} else {
									$spl ( $ex, $profile ['object_to_proper_attribute'], $in );
								}
							}
						}
						// proper object to proper attribute
						if (in_array ( $in, $c ['properIntent'] ) && in_array ( $ex, $c ['properExtent'] )) {
							if (! $skip ['proper_object_to_proper_attribute']) {
								if ($types ['attribute'] == 'uri') {
									$in = ($addns ['attribute']) ? $ns . $in : $in;
									$spo ( $ex, $profile ['proper_object_to_proper_attribute'], $in );
								} else {
									$spl ( $ex, $profile ['proper_object_to_proper_attribute'], $in );
								}
							}
						}
						
					}
				}
			}
		}
		
		ob_end_flush ();
	}
	
	public function postexportAction() {
		$taxonomy = $this->get ( 'taxonomy' );
		$profile = $this->get ( 'profile' );
		if ($profile == 0) {
			$profile = $this->_profile ();
		} else {
			$db = DataAccess::instance ();
			$p = $db->profile ( $profile );
			$profile = unserialize ( $p ['data'] );
		}
		$this->doexport ( $taxonomy, $profile );
	}
	public function postprofileAction() {
		// TODO CHECK IF POST
		// TODO VALIDATION
		$id = $this->get ( 'id', 0 );
		$name = $this->get ( 'name', '' );
		$description = $this->get ( 'description', '' );
		$profile = array ();
		$profile ['topclass'] = $this->get ( 'topclass', NULL );
		$profile ['subClassOf'] = $this->get('subClassOf', NULL);
		$profile ['class_to_attribute'] = $this->get ( 'class_to_attribute', NULL );
		$profile ['class_to_proper_attribute'] = $this->get ( 'class_to_proper_attribute', NULL );
		$profile ['class_to_object'] = $this->get ( 'class_to_object', NULL );
		$profile ['class_to_proper_object'] = $this->get ( 'class_to_proper_object', NULL );
		$profile ['object_to_class'] = $this->get ( 'object_to_class', NULL );
		$profile ['proper_object_to_class'] = $this->get ( 'proper_object_to_class', NULL );
		$profile ['object_to_attribute'] = $this->get ( 'object_to_attribute', NULL );
		$profile ['object_to_proper_attribute'] = $this->get ( 'object_to_proper_attribute', NULL );
		$profile ['proper_object_to_attribute'] = $this->get ( 'proper_object_to_attribute', NULL );
		$profile ['proper_object_to_proper_attribute'] = $this->get ( 'proper_object_to_proper_attribute', NULL );
		$profile ['namespace'] = $this->get ( 'namespace', NULL );
		$profile ['skip'] = $this->get ( 'skip', array () );
		$profile ['types'] = $this->get ( 'types', array () );
		$profile ['addns'] = $this->get ( 'addns', array () );
		$profile = $this->_profile ( $profile );
		$db = DataAccess::instance ();
		if ($id == 0) {
			$id = $db->insertExportprofile ( $name, $description, serialize ( $profile ) );
			ACL::grant ( ACL::R_ACTIVE, ACL::P_READ, ACL::EXPORTPROFILE, $id );
			ACL::grantUser ( $this->_userid (), ACL::P_WRITE, ACL::EXPORTPROFILE, $id );
			redirect ( route ( 'export', 'list' ) );
		} else {
			if (ACL::allowed ( $this->_userid (), ACL::P_WRITE, ACL::EXPORTPROFILE, $id )) {
				$db->updateExportprofile ( $id, $name, $description, serialize ( $profile ) );
				redirect ( route ( 'export', 'list' ) );
			} else {
				$this->_forbidden ( 'Cannot edit export profile.' );
			}
		}
	}
	public function deleteprofileAction() {
		$db = DataAccess::instance();
		$id = $this->get ( 'id' );
		if (ACL::allowed ( $this->_userid (), ACL::P_WRITE, ACL::EXPORTPROFILE, $id )) {
			$db->deleteExportprofile ( $id );
			redirect ( route ( 'export', 'list' ) );
		} else {
			$this->_forbidden ( 'Cannot edit export profile.' );
		}
	}
}