<?php
// $Id: media_youtube.admin.inc,v 1.1.2.7 2010/04/21 14:59:59 aaron Exp $

/**
 *  @file
 *  Administrative page callbacks for Media: YouTube.
 */

/**
 *  Callback for /media/add/media_youtube/upload
 */
function media_youtube_upload() {
  if($_GET['status'] == '200' && isset($_GET['id']) && strlen($_GET['id']) > 0) {
	$uri = 'youtube://' . 'v/' . $_GET['id'];
    // Check to see if this a duplicate of an existing file
    $files = file_load_multiple(NULL, array('uri' => $uri));
    if ($files) {
      // This is ugly.
      $file = array_shift($files);
    }
    else {
      // @TODO: This won't work for YouTube and many other streams.
      // copy($url, $destination);
      $file = file_uri_to_object($uri);
      file_save($file);
    }

    // field_attach_insert('media', $file);
    if ($file) {
      field_attach_submit('media', $file, $form, $form_state);
      field_attach_insert('media', $file);
      drupal_goto('media/' . $file->fid . '/edit');
    }
    else {
        drupal_set_message(t('An error occurred. The file was uploaded but not saved into the CMS.'), 'error');
        drupal_goto('media/add/media_youtube');
    }
  }
  else { 
    return theme('media_youtube_upload', array('url' => $_GET['url'], 'token' => $_GET['token']));
  }
}

/**
 *  Callback for /media/add/media_youtube and
 *  /admin/content/media/add/media_youtube.
 */
function media_youtube_add($form, &$form_state = array(), $redirect = NULL) {
  global $user;

  $form = array();
  $form['youtube'] = array(
    '#type' => 'vertical_tabs',
  );
  $form['youtube']['all'] = array(
    '#type' => 'fieldset',
    '#title' => t('All YouTube videos'),
  );

  // Get all youtube files for this user
  $results = db_query("SELECT fid FROM {file_managed} WHERE uid = :uid AND uri LIKE :uri", array(
    ':uid' => $user->uid,
    ':uri' => 'youtube%%'
  ))->fetchAll();

  foreach ($results as $result) {
    $file = file_load($result->fid);
    $output = theme('image', array(
      'path' => 'http://img.youtube.com/vi/' . pathinfo($file->uri, PATHINFO_FILENAME) . '/0.jpg',
      'title' => 'title',
      'alt' => 'alt',
      'attributes' => array('width' => 150),
      'getsize' => FALSE,
    ));
    $form['youtube']['all'][$file->fid] = array(
      '#markup' => $output,
    );
  }

/*  $form['youtube']['all']['test'] = array(
    '#type' => 'checkbox',
    '#title' => 'test',
  );*/
  $form['youtube']['add_from_url'] = array(
    '#type' => 'fieldset',
    '#title' => t('Add from URL'),
  );
  $form['youtube']['add_from_url']['url'] = array(
    '#type' => 'textfield',
    '#title' => 'URL',
    '#description' => 'Input the URL of the desired YouTube video page.',
  );

  $form['youtube']['browser_upload'] = array(
	'#type' => 'fieldset',
	'#title' => t('Upload'),
  );
  $form['youtube']['browser_upload']['title'] = array(
    '#type' => 'textfield',
    '#title' => t('Title'),
	'#required' => true,
  );
  $form['youtube']['browser_upload']['description'] = array(
	'#type' => 'textfield',
	'#title' => t('Description'),
	'#required' => true,
  );
  $form['youtube']['browser_upload']['category'] = array(
	'#type' => 'textfield',
	'#title' => t('Category'),
	'#required' => true,
  );
  $form['youtube']['browser_upload']['tags'] = array(
	'#type' => 'textfield',
	'#title' => t('Tags'),
	'#required' => true,
  );

  $form['redirect'] = array(
    '#type' => 'value',
    '#value' => $redirect,
  );

  $form['submit'] = array(
    '#type' => 'submit',
    '#value' => 'Submit',
  );

  return $form;
}

/**
 *  Validation for media_youtube_add().
 */
function media_youtube_add_validate($form, &$form_state) {
  if($form_state['values']['youtube__active_tab'] != 'edit-browser-upload') {
  	if (!preg_match('@youtube\.com/watch\?v=([^"\& ]+)@i', $form_state['values']['url'], $matches)) {
    	form_set_error('url', t('Please submit a valid YouTube video URL.'));
  	}
  }
  else {
	if(empty($form_state['values']['title'])) {
		form_set_error('title', t('Please submit a title'));
	}
	if(empty($form_state['values']['description'])) {
		form_set_error('description', t('Please submit a description'));
	}
	if(empty($form_state['values']['category'])) {
		form_set_error('category', t('Please submit a category'));
	}
	if(empty($form_state['values']['tags'])) {
		form_set_error('tags', t('Please submit at least one tag'));
	}
  }
}

/**
 *  Submission for media_youtube_add().
 *
 *  This will create a file object for the YouTube video.
 */
function media_youtube_add_submit($form, &$form_state) {
	if(!empty($form_state['values']['youtube__active_tab']) == 'edit-browser-upload') {
			
	  // get authentication token
      $url_auth = 'https://www.google.com/youtube/accounts/ClientLogin';
      $url_token = 'http://gdata.youtube.com/action/GetUploadToken';
      $username = 'chaptermedia';
      $password = 'tatt00tatt00';
      $developer_key = 'AI39si6R6aDmdDejMYuIyfBxhLWY_rGn3Iu6iUd9S6Lam703eXPnQT72YeR6ouIJiuAudT2NLNTJrs7iK3crPF2vAMowQL0TmA';

      $ch = curl_init($url_auth);
      curl_setopt($ch, CURLOPT_SSL_VERIFYPEER, FALSE);
      curl_setopt($ch, CURLOPT_POST, 1);
      curl_setopt($ch, CURLOPT_POSTFIELDS, "Email=$username&Passwd=$password&service=youtube&source=DrupalMedia");
      curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1); 
      $response = curl_exec($ch);
      $status = curl_getinfo($ch, CURLINFO_HTTP_CODE); 
      curl_close($ch);

      if($status == 200) {

        // Parse Google's response
        $goog_resp = array();
        foreach (explode("\n", $response) as $l) {
          $l = chop($l);
          if ($l) {
            list($key, $val) = explode('=', chop($l), 2);
            $goog_resp[$key] = $val;
          }
        }

        $API_XML_Request =
<<<EOT
<?xml version="1.0"?>
        <entry xmlns="http://www.w3.org/2005/Atom"
          xmlns:media="http://search.yahoo.com/mrss/"
          xmlns:yt="http://gdata.youtube.com/schemas/2007">
          <media:group>
            <media:title type="plain">{$form_state['values']['title']}</media:title>
            <media:description type="plain">{$form_state['values']['description']}</media:description>
            <media:category scheme="http://gdata.youtube.com/schemas/2007/categories.cat">{$form_state['values']['category']}</media:category>
            <media:keywords>{$form_state['values']['tags']}</media:keywords>
          </media:group>
        </entry>
EOT;

        $headers = array(
          'Authorization: GoogleLogin auth=' . $goog_resp['Auth'],
          'GData-Version: 2',
          'X-GData-Key: key=' . $developer_key . '',
          'Content-Length: ' . mb_strlen($API_XML_Request, 'utf8'),
          'Content-Type: application/atom+xml; charset=UTF-8',
        );

        $ch = curl_init($url_token);
        curl_setopt($ch, CURLOPT_HTTPHEADER, $headers);
        curl_setopt($ch, CURLOPT_POST, 1);
        curl_setopt($ch, CURLOPT_POSTFIELDS, $API_XML_Request);
        curl_setopt($ch, CURLOPT_RETURNTRANSFER, 1); 
        $response = curl_exec($ch);
        $status = curl_getinfo($ch, CURLINFO_HTTP_CODE); 
        curl_close($ch);

        if($status == 200) {
          $xml = new SimpleXMLElement($response);
          $form_state['redirect'] = array('media/add/media_youtube/upload', array(
	        'query' => array(
		      'url' => $xml->url,
		      'token' => $xml->token,
		    )
	      ));
        }
        else {
          $form_state['rebuild'] = TRUE;
          $xml = new SimpleXMLElement($response);
	      drupal_set_message('Error: ' . $xml->error[0]->code . ' : ' . $xml->error[0]->location, 'error');
	    }
	  }
	  else {
		drupal_set_message(t('Error authenticating with YouTube, check your settings'), 'error');
	  }
    }
	else {
	
	  $defaults = array (
	    'display' => TRUE,
	  );

	  $uri = media_youtube_media_parse($form_state['values']['url']);

	  // Check to see if this a duplicate of an existing file
	  $files = file_load_multiple(NULL, array('uri' => $uri));
	  if ($files) {
	    // This is ugly.
	    $file = array_shift($files);
	  }
	  else {
	    // @TODO: This won't work for YouTube and many other streams.
	    // copy($url, $destination);
	    $file = file_uri_to_object($uri);
	    file_save($file);
	  }

	  // field_attach_insert('media', $file);
	  if ($file) {
	    $form_state['redirect'] = 'media/' . $file->fid . '/edit';
	    field_attach_submit('media', $file, $form, $form_state);
	    field_attach_insert('media', $file);
	  }
	  else {
	    drupal_set_message(t('An error occurred and no file was saved.'), 'error');
	  }

	  $form_state['redirect'] = !empty($form_state['values']['redirect']) ? $form_state['values']['redirect'] : 'media/' . $file->fid . '/edit';
	}
}
