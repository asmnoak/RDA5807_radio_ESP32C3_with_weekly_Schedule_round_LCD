<p><H3>�T�ԃX�P�W���[�����\��RDA5807 FM DSP���W�I�i�ی`LCD���p�o�[�W�����j</H3></p>
<p>
�\����Ɋی`�J���[LCD���p���g�p���A�T�ԃX�P�W���[�����\��FM���W�I�𐻍삵���̂ŏЉ��B<br>
�ی`�J���[LCD�́A1.28�C���`�A240x240�A�h���C�o�[��CG9A01�ł���B<br>
DSP���W�IIC��RDA5807FP�AI2C�C���^�[�t�F�[�X�ɂ��R���g���[���iMCU�j��Seeed Studio XIAO ESP32C3�ł���B<br>
XIAO ESP32C3�ł́AClock�̐ݒ��NTP�𗘗p���A������Web�T�[�o�[�@�\�ŁA����^�T�ԃX�P�W���[���̐ݒ���s���Ă���B<br>
�J����Arduino IDE 1.8�ōs�����B<br>
�ی`�J���[LCD�ȊO�̊�{�I�Ȃ��Ƃ́A<a href="https://github.com/asmnoak/RDA5807_radio_ESP32C3_with_weekly_Schedule">������</a>���Q�Ƃ̂��ƁB
</p>

<p><strong>�T�v</strong><br>
�ی`�J���[LCD�́ASPI�Ő��䂷��̂ŁAI2C��SPI�𓯎��ɗ��p���邱�ƂɂȂ�B���̏ꍇ�AXIAO ESP32C3�͓��o�̓s���̐���������̂ŁA<br>
����̓{�^���X�C�b�`�ł͂Ȃ��AWiFi�o�R�ōs�Ȃ����ƂɂȂ�B<br>
CG9A01���T�|�[�g���郉�C�u������<a href="https://github.com/Bodmer/TFT_eSPI">"TFT_eSPI.h"</a>�𗘗p�����B
<p>TFT_eSPI�͎��O�ɁA�g�p����h���C�o��MCU�ɑΉ����邽�߂̕ύX���K�v�ł���B���C�u�������C���X�g�[��������A�u..\Arduino\libraries\TFT_eSPI�v�́uUser_Setup.h�v��ҏW����B<br>
�ҏW��̃t�@�C�����uUser_Setup.h.mod�v�ł���B</p>
<ol>
<li>�uSection 1. Call up the right driver file and any options for it�v�̌��ɁA�f�B�X�v���C�Ή��̃h���C�o�̒�`������̂ŁA�u#define ILI9341_DRIVER�v�̍s���R�����g�����A�u#define GC9A01_DRIVER�v�̍s���u��v�R�����g������B</li>
<li>�uSection 2. Define the pins that are used to interface with the display here�v�̌��ɁA�eMCU����SPI�ɑ΂���s���̒�`������̂ł����ύX����B�܂��AESP8266�̃s����`���R�����g������B<br>
���ɁAXIAO_ESP32C3�ɑ΂���s����`��ǉ�����B�ǉ����e�́A�uUser_Setup.h.mod�v���Q�Ƃ̂��ƁB</li>
</ol>
</p>
<p><strong>H/W�\��</strong><br>
 �ESeeed Studio XIAO ESP32C3 - �R���g���[��<br>
 �EI2C�ڑ�&nbsp; RDA5807FP<br>
 �ESPI�ڑ�&nbsp; �ی`�J���[LCD�\�����u(CG9A01)<br>
 �EXtal���U��i32768Hz�j�A�R���f���T�ށA�I�[�f�B�I�W���b�N�A�z����<br>
</p>
<p>
<img src="./CG9A01_RDA5807_XIAO_1.jpg" width="440" height="360"><br>
�������ی`�J���[LCD�A�E����XIAO ESP32C��RDA5807FP�̊�B�Ȃ��ABLK�i�o�b�N���C�g����j�͐ڑ����Ă��Ȃ��B
</p>
<p><strong>�ڑ�</strong><br>
�e�R���|�[�l���g�̐ڑ��͈ȉ��̒ʂ�B<br>
<p>
<table> 
<tr>
<td>I2C&nbsp;</td><td>XIAO</td>
</tr>
<tr>
<td>SCK</td><td>D5</td>
<tr>
<tr>
<td>SDA</td><td>D4</td>
<tr>
</table>
</p>
<p>
<table> 
<tr>
<td>SPI</td><td>XIAO</td>
</tr>
<tr>
<td>SDA</td><td>GPI10(MOSI)</td>
</tr>
<tr>
<td>SCL</td><td>GPIO8(SCK)</td>
</tr>
<tr>
<td>CS</td><td>GPIO3</td>
</tr>
<tr>
<td>DC</td><td>GPIO5</td>
</tr>
<tr>
<td>RES</td><td>GPIO4</td>
</tr>
</table>
</p>
</p>
<p>
I2C�̃A�h���X
<table> 
<tr>
<td>RDA5807FP</td><td>0x10&nbsp;or&nbsp;0x11&nbsp;���C�u�����Ŋ���</td>
</tr>
</table>
</p>
<p><strong>�C���X�g�[��</strong><br>
<ol>
<li>�R�[�h���AZIP�`���Ń_�E�����[�h</li>
<li>�ǉ��̃��C�u�������AZIP�`���Ń_�E�����[�h�A���C�u�����}�l�[�W������C���X�g�[������</li>
 <ul>
  <li>RDA5807�i���C�u�����}�l�[�W�����猟���j</li>
  <li>TFT_eSPI</li>
  <li>TimeLib&nbsp;:&nbsp; https://github.com/PaulStoffregen/Time</li>
 </ul>
<li>ArduinoIDE����xiao_esp32c3_cg9a01_rda5801_wifi_clock_radio.ino���J��</li>
<li>�u���؁E�R���p�C���v�ɐ���������A��U�A�u���O��t���ĕۑ��v���s��</li>
</ol>
</p>
<p><strong>�኱�̉��</strong><br>
�E�R�[�h����WiFi��SSID��PASSWORD�́A���p���Ă��閳�����[�^�[�̂��̂��w�肷��B<br>
�E�܂��AstnFreq[]�Ɏ�M��������ǂ̎��g�����w�肷��B�Ⴆ��80.4MHz�̏ꍇ�A8040�Ǝw�肷��BstnName[]�ɂ͑Ή���������ǖ����w�肷��B<br>
�E"Free_Fonts.h"��TFT_eSPI�̃T���v�����R�s�[�������̂ł���B�t�H���g�̎��̂�TFT_eSPI���C�u�������ɑ��݂���B<br>
�E�O������鉩�F�̊ۈ�́A�b�j�̑ւ��ł���B<br>
<p>
<img src="./CG9A01_RDA5807_XIAO_2.jpg" width="360" height="360"><br>
</p>
</p>
<p><strong>���ӎ���</strong><br>
�E���p�̍ۂ́A���ȐӔC�ł��y���݂��������B<br>
</p>
