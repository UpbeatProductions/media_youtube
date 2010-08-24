<?php
// $Id: MediaYouTubeStreamWrapper.inc,v 1.1.2.2 2009/11/10 21:10:03 aaron Exp $

/**
 *  @file
 *  Create a YouTube Stream Wrapper class for the Media/Resource module.
 */

/**
 *  Create an instance like this:
 *  $youtube = new ResourceYouTubeStreamWrapper('youtube://?v=[video-code]');
 */
class MediaYouTubeStreamWrapper extends MediaReadOnlyStreamWrapper {
  protected $base_url = 'http://youtube.com/watch';
}
