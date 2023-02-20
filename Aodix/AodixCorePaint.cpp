/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Aodix Core Paint Implementation
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
#include "./aodixcore.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
char note_name[128][4]=
{
	"C-0","C#0","D-0","D#0","E-0","F-0","F#0","G-0","G#0","A-0","A#0","B-0",
	"C-1","C#1","D-1","D#1","E-1","F-1","F#1","G-1","G#1","A-1","A#1","B-1",
	"C-2","C#2","D-2","D#2","E-2","F-2","F#2","G-2","G#2","A-2","A#2","B-2",
	"C-3","C#3","D-3","D#3","E-3","F-3","F#3","G-3","G#3","A-3","A#3","B-3",
	"C-4","C#4","D-4","D#4","E-4","F-4","F#4","G-4","G#4","A-4","A#4","B-4",
	"C-5","C#5","D-5","D#5","E-5","F-5","F#5","G-5","G#5","A-5","A#5","B-5",
	"C-6","C#6","D-6","D#6","E-6","F-6","F#6","G-6","G#6","A-6","A#6","B-6",
	"C-7","C#7","D-7","D#7","E-7","F-7","F#7","G-7","G#7","A-7","A#7","B-7",
	"C-8","C#8","D-8","D#8","E-8","F-8","F#8","G-8","G#8","A-8","A#8","B-8",
	"C-9","C#9","D-9","D#9","E-9","F-9","F#9","G-9","G#9","A-9","A#9","B-9",
	"C-A","C#A","D-A","D#A","E-A","F-A","F#A","G-A"
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::paint_master_vumeter(HDC const hdc)
{
	// num peaks
	int const num_peaks=dsp_highest_opin_count+1;

	// vumeter height
	int const vumeter_height=64/num_peaks;

	// dynamic range (in db)
	float const db_range=84.0f;

	// scan peaks
	for(int pk=0;pk<num_peaks;pk++)
	{
		// get linear peak level
		float const linear_peak_level=dsp_output_vumeter[pk];

		// vumeter peak width
		int vumeter_width=0;

		// get db magnitude
		if(linear_peak_level>0.0f)
		{
			// get peak graphic magnitude
			float const peak_db=20.0f*log10f(linear_peak_level);
			float const peak_wd=(db_range+peak_db)*(213.0f/db_range);

			// set vu width
			vumeter_width=arg_tool_clipped_assign(peak_wd,0,213);
		}

		// vumeter y position
		int const vu_y=31+(vumeter_height*pk);

		// paint vumeter bar
		if(vumeter_width>0)
			BitBlt(hdc,798,vu_y,vumeter_width,vumeter_height-1,hdc_gfx,64,304,SRCCOPY);
		
		// paint remaining bar
		if(vumeter_width<213)
			BitBlt(hdc,798+vumeter_width,vu_y,213-vumeter_width,vumeter_height-1,hdc_gfx,64+vumeter_width,336,SRCCOPY);

		// clipping peak led
		BitBlt(hdc,1012,vu_y,4,vumeter_height-1,hdc_gfx,278,336-(linear_peak_level>1.0f)*32,SRCCOPY);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::paint_block_shade(HDC const hdc,int const x,int const y,int const w,int const h,int const lx1,int const ly1,int const lx2,int const ly2)
{
	// alphablend structure
	BLENDFUNCTION bf;

	// fill ab struct
	bf.AlphaFormat=0;
	bf.BlendFlags=0;
	bf.BlendOp=0;
	bf.SourceConstantAlpha=64;

	// get rectangle coordinates
	int x1=x;
	int y1=y;
	int x2=x+w;
	int y2=y+h;

	// clamp rectangle coordinates
	if(x1<lx1)
		x1=lx1;

	if(y1<ly1)
		y1=ly1;

	if(x2>lx2)
		x2=lx2;

	if(y2>ly2)
		y2=ly2;

	// get coordinates
	int const i_bm_x=x1;
	int const i_bm_y=y1;
	int const i_bm_w=x2-x1;
	int const i_bm_h=y2-y1;

	// check block size and mark
	if(i_bm_w>0 && i_bm_h>0)
	{
		// frame
		arg_gdi_paint_solid_rect(hdc,i_bm_x,i_bm_y,i_bm_w,1,0x00FFFFFF);
		arg_gdi_paint_solid_rect(hdc,i_bm_x,i_bm_y,1,i_bm_h,0x00FFFFFF);
		arg_gdi_paint_solid_rect(hdc,i_bm_x+1,i_bm_y+i_bm_h-1,i_bm_w-1,1,0x00FFFFFF);
		arg_gdi_paint_solid_rect(hdc,i_bm_x+i_bm_w-1,i_bm_y+1,1,i_bm_h-1,0x00FFFFFF);

		// alphablend edit block shade
		AlphaBlend(hdc,i_bm_x,i_bm_y,i_bm_w,i_bm_h,hdc_gfx,256,0,16,16,bf);
	}
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::paint_txt(HDC const hdc,int const x,int const y,char* txt,int const num_chars,int const type)
{
	// set source type origin
	int const yo=588+type*8;

	// blit char string
	for(int c=0;c<num_chars;c++)
		BitBlt(hdc,x+c*6,y,6,8,hdc_gfx,txt[c]*6,yo,SRCCOPY);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::paint_seq_pos_big(HDC const hdc,int const x,int const y,int const pos)
{
	// get bar, beat and ppq position
	int const bar=pos/(project.master_numerator*project.master_ppqn);
	int const bea=(pos/project.master_ppqn)%project.master_numerator;
	int const ppq=pos%project.master_ppqn;

	// get bar digits
	int const bar_dig_c=bar/100;
	int const bar_dig_d=(bar/10)%10;
	int const bar_dig_u=bar%10;

	// get peak digits
	int const ppq_dig_c=ppq/100;
	int const ppq_dig_d=(ppq/10)%10;
	int const ppq_dig_u=ppq%10;

	// blit digits
	BitBlt(hdc,x+00,y,12,16,hdc_gfx,64+bar_dig_c*12,80,SRCCOPY);
	BitBlt(hdc,x+13,y,12,16,hdc_gfx,64+bar_dig_d*12,80,SRCCOPY);
	BitBlt(hdc,x+26,y,12,16,hdc_gfx,64+bar_dig_u*12,80,SRCCOPY);
	BitBlt(hdc,x+38,y,12,16,hdc_gfx,184,80,SRCCOPY);
	BitBlt(hdc,x+50,y,12,16,hdc_gfx,64+bea*12,80,SRCCOPY);
	BitBlt(hdc,x+62,y,12,16,hdc_gfx,184,80,SRCCOPY);
	BitBlt(hdc,x+74,y,12,16,hdc_gfx,64+ppq_dig_c*12,80,SRCCOPY);
	BitBlt(hdc,x+87,y,12,16,hdc_gfx,64+ppq_dig_d*12,80,SRCCOPY);
	BitBlt(hdc,x+100,y,12,16,hdc_gfx,64+ppq_dig_u*12,80,SRCCOPY);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::paint_event(HDC const hdc,int const x,int const y,int const w,int const h,COLORREF const text_color,COLORREF const back_color,char* text)
{
	// background
	arg_gdi_paint_solid_rect(hdc,x,y,w,h,back_color);

	// big event frame
	arg_gdi_paint_solid_rect(hdc,x,y,1,1,back_color+ARG_GDI_3D_CLR_OFFSET+ARG_GDI_3D_CLR_OFFSET);
	arg_gdi_paint_solid_rect(hdc,x+1,y,w-2,1,back_color+ARG_GDI_3D_CLR_OFFSET);
	arg_gdi_paint_solid_rect(hdc,x,y+1,1,h-2,back_color+ARG_GDI_3D_CLR_OFFSET);
	arg_gdi_paint_solid_rect(hdc,x+1,y+h-1,w-2,1,back_color-ARG_GDI_3D_CLR_OFFSET);
	arg_gdi_paint_solid_rect(hdc,x+w-1,y+1,1,h-2,back_color-ARG_GDI_3D_CLR_OFFSET);
	arg_gdi_paint_solid_rect(hdc,x+w-1,y+h-1,1,1,back_color-ARG_GDI_3D_CLR_OFFSET-ARG_GDI_3D_CLR_OFFSET);

	// paint event text
	SetTextColor(hdc,text_color);
	SetBkColor(hdc,back_color);

	// text out event
	TextOut(hdc,x+3,y+2,text,15);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::paint_marker(HDC const hdc,COLORREF const color,int const type,char* label,int const y,int const w,int const label_offset,int const selected)
{
	// paint marker dotted line
	for(int x=0;x<w;x+=4)
		arg_gdi_paint_solid_rect(hdc,x,y,2,1,color);

	// paint marker
	BitBlt(hdc,0,y+label_offset,TRACK_WIDTH,16,hdc_gfx,320+selected*TRACK_WIDTH,368+type*16,SRCCOPY);

	// paint label
	paint_txt(hdc,38,y+label_offset+4,label,9,selected*4);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::paint_wire(HDC const hdc,int const x1,int const y1,int const x2,int const y2,COLORREF const clr)
{	
	// get vector 
	double const d_vw=double(x2-x1);
	double const d_vh=double(y2-y1);

	// get modulus
	double const d_mo=sqrt(d_vw*d_vw+d_vh*d_vh);

	// paint line wire
	arg_gdi_paint_line(hdc,x1,y1,x2,y2,PS_SOLID,clr);

	// is modulus higher than 1.0
	if(d_mo>1.0)
	{
		// get normalized values
		double const d_xr=(d_vw/d_mo)*8.0;
		double const d_yr=(d_vh/d_mo)*8.0;

		// get center
		double const d_cx=double(x1)+d_vw*0.5;
		double const d_cy=double(y1)+d_vh*0.5;

		// polygon points
		POINT pnt[3];

		// get the three arrow coordinates
		pnt[0].x=d_cx+d_xr;
		pnt[0].y=d_cy+d_yr;
		pnt[1].x=d_cx+d_yr;
		pnt[1].y=d_cy-d_xr;
		pnt[2].x=d_cx-d_yr;
		pnt[2].y=d_cy+d_xr;

		// point polygon
		Polygon(hdc,pnt,3);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::paint_pin_array(HDC const hdc,int const x,int const y,int const num_pins,ADX_PIN* ppin,int const master_o_x,int const master_o_y,int const rout_area_y)
{
	// paint pins
	for(int p=0;p<num_pins;p++)
	{
		// get pin pointer
		ADX_PIN* pp=&ppin[p];

		// scan wire
		for(int w=0;w<pp->num_wires;w++)
		{
			// get wire pointer
			ADX_WIRE* pw=&pp->pwire[w];

			// get wire colour
			int const clr_scl=int(pw->value*255.0f);

			// wire colour code
			COLORREF const clr_code=clr_scl*0x00010101;

			// create and select the brush that will be used to fill the polygon
			HBRUSH const hbrush=CreateSolidBrush(clr_code);
			
			// select brush
			HBRUSH const hbrush_old=(HBRUSH)SelectObject(hdc,hbrush);

			// wire connected to master output, paint wire
			if(pw->instance_index==MAX_INSTANCES)
				paint_wire(hdc,x+p*8+4,y,master_o_x+pw->pin_index*8+4,master_o_y+4,0);

			// wire connected to machine
			if(pw->instance_index<MAX_INSTANCES)
			{
				// get dest machine
				ADX_INSTANCE* pi=&instance[pw->instance_index];

				// check if effect is loaded
				if(pi->peffect!=NULL)
				{
					// get instance module screen coordinates
					int const i_x=0+pi->x-user_rout_offset_x;
					int const i_y=rout_area_y+pi->y-user_rout_offset_y;

					// paint wire
					paint_wire(hdc,x+p*8+4,y,i_x+4+pw->pin_index*8,i_y+4,0);
				}
			}

			// select old hbrush object
			SelectObject(hdc,hbrush_old);

			// delete brush
			DeleteObject(hbrush);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::paint_track_midi_vumeters(HDC const hdc,int const x,int const y,int const visible_tracks)
{
	// draw track frames
	for(int t=0;t<visible_tracks;t++)
	{
		// get track index
		int const ti=user_trk_offset+t;

		// decide whenever track is visible
		if(ti<MAX_TRACKS)
		{
			// blit vumeter bar
			BitBlt(hdc,x+34+t*TRACK_WIDTH,y,4,12,hdc_gfx,256+(master_midi_vumeter[ti]/8)*4,80,SRCCOPY);

			// decay track vumeter
			master_midi_vumeter[ti]=arg_tool_clipped_assign(master_midi_vumeter[ti]-6,0,128);
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void CAodixCore::paint(HWND const hwnd,HDC const hdc,int const w,int const h)
{
	// temporal text buffer holder
	char buf_a[256];

	// back color
	COLORREF const bck_color=GetPixel(hdc_gfx,0,0);

	// alphablend structure
	BLENDFUNCTION bf;
	bf.AlphaFormat=0;
	bf.BlendFlags=0;
	bf.BlendOp=0;

	// instance current selected instance pointer
	ADX_INSTANCE* pi=&instance[user_instance];

	// get current pattern pointer
	ADX_PATTERN* pp=&project.pattern[user_pat];

	// get user quantization
	int const i_quantize=edit_get_quantization();

	// get master pos
	int const i_master_play_pos=seq_sample_to_pos(master_transport_sampleframe);

	// set font, get default font
	HFONT const hfont_default=(HFONT)SelectObject(hdc,(HFONT)hfont_terminal);

	// set transparent text mode
	SetBkMode(hdc,TRANSPARENT);

	// master timer		
	gui_format_time(master_transport_sampleframe,buf_a);
	paint_txt(hdc,135,21,buf_a,9,2);

	// master cpu
	sprintf(buf_a,"%.2d.%.1d",int(dsp_cpu_cost),int(dsp_cpu_cost*10.0)%10);
	paint_txt(hdc,159,37,buf_a,4,2);

	// master transport cycle switch
	BitBlt(hdc,195,17,32,16,hdc_gfx,64,240+(user_pressed==1)*16+(master_time_info.flags & kVstTransportCycleActive)*8,SRCCOPY);

	// master config rec-live switch
	BitBlt(hdc,227,17,32,16,hdc_gfx,96,240+(user_pressed==2)*16+cfg.rec_live*32,SRCCOPY);

	// master config stop wrap
	BitBlt(hdc,195,33,64,16,hdc_gfx,128,240+(user_pressed==3)*16+cfg.stop_wrap*32,SRCCOPY);

	// master tempo
	sprintf(buf_a,"%.3d.%.1d",int(project.master_tempo),int(project.master_tempo*10.0)%10);
	paint_txt(hdc,71,53,buf_a,5,0);
	BitBlt(hdc,115,49,16,16,hdc_gfx,16+(user_pressed==4)*16,32,SRCCOPY);

	// master ppqn
	sprintf(buf_a,"%.3d",project.master_ppqn);
	paint_txt(hdc,199,53,buf_a,3,0);

	// master signature
	sprintf(buf_a,"%d:%d",project.master_numerator,project.master_denominator);
	paint_txt(hdc,71,69,buf_a,3,0);

	// master block size
	sprintf(buf_a,"%.6d",dsp_block_size);
	paint_txt(hdc,199,69,buf_a,6,0);

	// master position
	paint_seq_pos_big(hdc,10,25,i_master_play_pos);

	// master transport play
	BitBlt(hdc,3,85,64,28,hdc_gfx,64,96+(user_pressed==7)*28+(master_time_info.flags & kVstTransportPlaying)*28,SRCCOPY);

	// master transport stop
	BitBlt(hdc,67,85,64,28,hdc_gfx,128,96+(user_pressed==8)*28,SRCCOPY);

	// master transport record events
	BitBlt(hdc,131,85,64,28,hdc_gfx,192,96+(user_pressed==9)*28+(master_time_info.flags & kVstTransportRecording)*7,SRCCOPY);

	// master transport record automation
	BitBlt(hdc,195,85,64,28,hdc_gfx,256,96+(user_pressed==10)*28+((master_time_info.flags & kVstAutomationWriting)>0)*56,SRCCOPY);

	// edit pos
	paint_seq_pos_big(hdc,10,141,pp->usr_pos);
	
	// edit timer
	gui_format_time(seq_pos_to_sample(pp->usr_pos),buf_a);
	paint_txt(hdc,135,137,buf_a,9,2);

	// edit seq num events
	sprintf(buf_a,"%.6d",seq_num_events);
	paint_txt(hdc,135,153,buf_a,6,2);

	// edit step switch
	BitBlt(hdc,195,133,32,16,hdc_gfx,192,240+(user_pressed==11)*16+user_edit_step*32,SRCCOPY);

	// edit overwrite switch
	BitBlt(hdc,227,133,32,16,hdc_gfx,224,240+(user_pressed==12)*16+user_edit_overwrite*32,SRCCOPY);

	// edit sequencer/routing page switch
	BitBlt(hdc,195,149,64,16,hdc_gfx,256,240+(user_pressed==13)*16+user_page*32,SRCCOPY);

	// edit quantization
	sprintf(buf_a,"%.4d",i_quantize);
	paint_txt(hdc,71,169,buf_a,4,0);

	// edit midi channel
	sprintf(buf_a,"%.1X (%.2d)",user_midi_ch,user_midi_ch+1);
	paint_txt(hdc,71,185,buf_a,6,0);

	// edit kbd note offset
	sprintf(buf_a,"%d (%.2d)",user_kbd_note_offset/12,user_kbd_note_offset);
	paint_txt(hdc,71,201,buf_a,6,0);

	// edit midi mask
	sprintf(buf_a,"%X",user_midi_mask);
	paint_txt(hdc,71,217,buf_a,2,0);

	// edit pattern sequencer zoom
	sprintf(buf_a,"1:%.3d",pp->usr_ppp);
	paint_txt(hdc,199,169,buf_a,5,0);
	BitBlt(hdc,243,165,16,16,hdc_gfx,16+(user_pressed==14)*16,32,SRCCOPY);

	// edit mode switch (notes or patterns)
	if(pp->usr_mod==0)
		paint_txt(hdc,199,185,"Event  ",7,0);
	else
		paint_txt(hdc,199,185,"Pattern",7,0);

	// edit mode switch button
	BitBlt(hdc,243,181,16,16,hdc_gfx,16+(user_pressed==15)*16,80,SRCCOPY);

	// edit note velocity
	sprintf(buf_a,"%.2X(%.3d)",user_kbd_velo,user_kbd_velo);
	paint_txt(hdc,199,201,buf_a,7,0);
	BitBlt(hdc,243,197,16,16,hdc_gfx,16+(user_pressed==16)*16,32,SRCCOPY);

	// edit note pre-release
	sprintf(buf_a,"%.4d",pp->usr_pre);
	paint_txt(hdc,199,217,buf_a,4,0);
	BitBlt(hdc,243,213,16,16,hdc_gfx,16+(user_pressed==17)*16,32,SRCCOPY);

	// edit display current instance index
	sprintf(buf_a,"%.2X",user_instance);
	paint_txt(hdc,333,201,buf_a,2,0);

	// edit display current instance alias
	paint_txt(hdc,353,201,pi->alias,19,0);

	// edit current instance spinners
	BitBlt(hdc,473,197,16,16,hdc_gfx,16+(user_pressed==18)*16,0,SRCCOPY);
	BitBlt(hdc,489,197,16,16,hdc_gfx,16+(user_pressed==19)*16,16,SRCCOPY);

	// edit current instance ghosted program spinners and program menu arrow
	BitBlt(hdc,473,213,16,16,hdc_gfx,48,0,SRCCOPY);
	BitBlt(hdc,489,213,16,16,hdc_gfx,48,16,SRCCOPY);
	BitBlt(hdc,505,213,16,16,hdc_gfx,48,48,SRCCOPY);

	// check if instance is loaded
	if(pi->peffect!=NULL)
	{
		// edit display current instance program
		sprintf(buf_a,"%.2X",pi->peffect->dispatcher(pi->peffect,effGetProgram,0,0,NULL,0.0f));
		paint_txt(hdc,333,217,buf_a,2,0);

		// get program name
		memset(buf_a,0,24);
		pi->peffect->dispatcher(pi->peffect,effGetProgramName,0,0,buf_a,0.0f);
		paint_txt(hdc,353,217,buf_a,19,0);

		// check number of programs
		if(pi->peffect->numPrograms>1)
		{
			// edit current instance program spinners and program menu arrow
			BitBlt(hdc,473,213,16,16,hdc_gfx,16+(user_pressed==20)*16,0,SRCCOPY);
			BitBlt(hdc,489,213,16,16,hdc_gfx,16+(user_pressed==21)*16,16,SRCCOPY);
			BitBlt(hdc,505,213,16,16,hdc_gfx,16,48,SRCCOPY);
		}
	}
	else
	{
		// no instance loaded (no program)
		paint_txt(hdc,333,217,"--",2,0);
		paint_txt(hdc,353,217,"---                ",19,0);
	}

	// blit instance list scrollbar
	BitBlt(hdc,265,17,16,176,hdc_gfx,48,96,SRCCOPY);
	BitBlt(hdc,265,17+(user_instance_list_offset*32)/49,16,16,hdc_gfx,16+(user_pressed==22)*16,96,SRCCOPY);
	
	// vst instance list entries
	for(int i=0;i<11;i++)
	{
		// get instance index
		int const ii=user_instance_list_offset+i;

		// instance selected flag
		int const isf=(ii==user_instance);

		// screen position
		int const iy=17+i*16;

		// get instance pointer
		pi=&instance[ii];

		// paint selected/unselected instance item list entry background
		BitBlt(hdc,281,iy,240,16,hdc_gfx,64,416-isf*16,SRCCOPY);

		// format instance index
		sprintf(buf_a,"%.2X",ii);
		paint_txt(hdc,284,iy+4,buf_a,2,isf);

		// paint alias
		paint_txt(hdc,304,iy+4,pi->alias,14,isf);

		// clear buf_a
		memset(buf_a,0,14);

		// get plugin program label (if instanced)
		if(pi->peffect!=NULL)
			pi->peffect->dispatcher(pi->peffect,effGetProgramName,0,0,buf_a,0.0f);
		else
			sprintf(buf_a,"---");

		// paint program label
		paint_txt(hdc,398,iy+4,buf_a,14,isf);

		// instance list mute
		if(pi->process_mute)
			BitBlt(hdc,489,iy,16,16,hdc_gfx,192,0,SRCCOPY);

		// instance list thru (bypass)
		if(pi->process_thru)
			BitBlt(hdc,505,iy,16,16,hdc_gfx,208,0,SRCCOPY);
	}

	// get current selected instance
	pi=&instance[user_instance];

	// check if instance is loaded
	if(pi->peffect!=NULL)
	{
		// blit parameter list scrollbar background
		BitBlt(hdc,527,17,16,176,hdc_gfx,48,96,SRCCOPY);

		// get parameter list slider screen position
		if(pi->peffect->numParams>11)
			BitBlt(hdc,527,17+(user_parameter_list_offset*160)/(pi->peffect->numParams-11),16,16,hdc_gfx,16+(user_pressed==23)*16,96,SRCCOPY);

		// display parameter(s) entries
		for(int pil=0;pil<11;pil++)
		{
			// get parameter index
			int const param_index=user_parameter_list_offset+pil;

			// get parameter y-screen coordinate
			int const iy=17+pil*16;		

			// get labels (if param is available)
			if(param_index<pi->peffect->numParams)
			{
				// parameter selected flag
				int const psf=(param_index==user_parameter);

				// blit parameter background
				BitBlt(hdc,543,iy,240,16,hdc_gfx,64,448-psf*16,SRCCOPY);

				// blit parameter index MSB
				sprintf(buf_a,"%.2X",param_index>>8);
				paint_txt(hdc,546,iy+4,buf_a,2,psf);

				// blit parameter index LSB
				sprintf(buf_a,"%.2X",param_index&0xFF);
				paint_txt(hdc,565,iy+4,buf_a,2,psf);

				// blit parameter name
				memset(buf_a,0,12);
				pi->peffect->dispatcher(pi->peffect,effGetParamName,param_index,0,buf_a,0.0f);
				paint_txt(hdc,584,iy+4,buf_a,11,psf);

				// blit parameter label
				memset(buf_a,0,12);
				pi->peffect->dispatcher(pi->peffect,effGetParamDisplay,param_index,0,buf_a,0.0f);
				paint_txt(hdc,660,iy+4,buf_a,11,psf);

				// blit parameter midi cc#
				if(pi->pmidi_cc[param_index]<255)
				{
					sprintf(buf_a,"#%.3d",pi->pmidi_cc[param_index]);
					paint_txt(hdc,739,iy+4,buf_a,4,psf);
				}

				// select knob frame
				float const f_frame=pi->peffect->getParameter(pi->peffect,param_index)*128.0f;
				int const i_frame=arg_tool_clipped_assign(int(f_frame),0,127);

				// draw parameter knob
				BitBlt(hdc,768,iy+1,14,14,hdc_knb,0,i_frame*14,SRCCOPY);
			}
			else
			{
				// out of param index
				BitBlt(hdc,543,iy,240,16,hdc_gfx,64,464,SRCCOPY);
			}
		}
	}
	else
	{
		// not instance loaded
		BitBlt(hdc,527,17,256,176,hdc_gfx,320,0,SRCCOPY);
	}

	// draw master output vumeter background
	BitBlt(hdc,789,17,232,92,hdc_gfx,64,480,SRCCOPY);

	// paint master vumeter
	paint_master_vumeter(hdc);

	// project information
	paint_txt(hdc,857,201,project.name,26,0);
	paint_txt(hdc,857,217,project.info,26,0);

	// midi-input information
	BitBlt(hdc,853,177,32,16,hdc_gfx,64+(p_hmidi_in!=NULL)*32,572,SRCCOPY);

	// last midi-in message monitor
	paint_txt(hdc,921,181,user_midi_in_monitor,8,0);

	// sequencer view
	if(user_page==0 && h>240)
	{
		// sequencer client area
		int const seq_area_y=264;
		int const seq_area_h=h-seq_area_y;
		int const seq_cent_y=seq_area_y+seq_area_h/2;
		int const seq_area_b=seq_area_y+seq_area_h;

		// number of visible tracks
		int const seq_visi_t=(w-user_pr_width)/TRACK_WIDTH;

		// track offset left fix
		if(user_trk<user_trk_offset)
			user_trk_offset=user_trk;

		// track offset right fix
		if(user_trk>=(user_trk_offset+(seq_visi_t-2)))
			user_trk_offset=user_trk-(seq_visi_t-2);

		// track clip region
		arg_gdi_set_clip_rgn(hdc,0,seq_area_y-32,w,32);

		// draw top-left pattern selector
		BitBlt(hdc,0,seq_area_y-32,TRACK_WIDTH,32,hdc_gfx,320,272,SRCCOPY);
		sprintf(buf_a,"%.2X",user_pat);
		paint_txt(hdc,32,236,buf_a,2,0);

		// draw pattern selector dec spin
		if(user_pressed==26)
			BitBlt(hdc,TRACK_WIDTH-48,seq_area_y-32,16,16,hdc_gfx,32,0,SRCCOPY);

		// draw pattern selector inc spin
		if(user_pressed==27)
			BitBlt(hdc,TRACK_WIDTH-32,seq_area_y-32,16,16,hdc_gfx,32,16,SRCCOPY);

		// paint pattern label
		paint_txt(hdc,4,252,pp->name,14,0);

		// draw piano-roll track
		BitBlt(hdc,TRACK_WIDTH,seq_area_y-32,user_pr_width-8,32,hdc_gfx,0,640,SRCCOPY);
		BitBlt(hdc,TRACK_WIDTH+user_pr_width-8,seq_area_y-32,8,32,hdc_gfx,760,640,SRCCOPY);

		// draw track frames
		for(int t=0;t<seq_visi_t;t++)
		{
			// get track index and screen coordinate
			int const ti=user_trk_offset+t;
			int const tx=TRACK_WIDTH+user_pr_width+t*TRACK_WIDTH;

			// decide whenever track is visible
			if(ti<MAX_TRACKS)
			{
				// get track pointer
				ADX_TRACK* pt=&pp->track[ti];

				// selected track flag
				int const stf=(ti==user_trk);

				// track frame
				BitBlt(hdc,tx,seq_area_y-32,TRACK_WIDTH,32,hdc_gfx,320+(stf*TRACK_WIDTH),304,SRCCOPY);

				// blit mute
				if(pt->mute)
					BitBlt(hdc,tx,seq_area_y-32,16,16,hdc_gfx,192,0,SRCCOPY);

				// blit solo
				if(pt->solo)
					BitBlt(hdc,tx+16,seq_area_y-32,16,16,hdc_gfx,224,0,SRCCOPY);

				// paint track index
				sprintf(buf_a,"%.3d",ti);
				paint_txt(hdc,tx+43,seq_area_y-28,buf_a,3,3+stf*2);

				// paint track label
				paint_txt(hdc,tx+4,seq_area_y-12,pt->name,14,stf*4);
			}
			else
			{
				// blank track
				BitBlt(hdc,tx,seq_area_y-32,TRACK_WIDTH,32,hdc_gfx,320,336,SRCCOPY);
			}
		}

		// paint track vumeters
		paint_track_midi_vumeters(hdc,TRACK_WIDTH+user_pr_width,seq_area_y-30,seq_visi_t);

		// clip sequencer region
		arg_gdi_set_clip_rgn(hdc,0,seq_area_y,w,seq_area_h);

		// fill sequencer background with solid color
		arg_gdi_paint_solid_rect(hdc,0,seq_area_y,w,seq_area_h,bck_color);

		// get gridlines viewport range
		int const i_bg_rng=(seq_area_h/2)*pp->usr_ppp;
		int const i_bg_min=pp->usr_pos-i_bg_rng;
		int const i_bg_max=pp->usr_pos+i_bg_rng;
		int const i_bg_div=project.master_ppqn/4;

		// gridline flags
		int const i_gridline_visible_tick=(pp->usr_ppp<=(project.master_ppqn/20));
		int const i_gridline_visible_beat=(pp->usr_ppp<=(project.master_ppqn/5));

		// gridline for each tick
		for(int bg=i_bg_min;bg<i_bg_max;bg+=i_bg_div)
		{
			int const bgi=bg/i_bg_div;
			int const tick_pos=bgi*i_bg_div;
			int const bgy=seq_cent_y+(tick_pos-pp->usr_pos)/pp->usr_ppp;

			// draw guide if visible
			if(bgi>=0)
			{
				// gridline label
				int const i_guid_seconds=seq_pos_to_sample(tick_pos)/int(cfg.asio_driver_sample_rate);
				sprintf(buf_a,"%.2d:%.2d   %.3d:%.1d",(i_guid_seconds/60)%60,i_guid_seconds%60,tick_pos/(project.master_ppqn*project.master_numerator),(tick_pos/project.master_ppqn)%project.master_numerator);

				// tick gridline
				if(i_gridline_visible_tick)
					arg_gdi_paint_solid_rect(hdc,TRACK_WIDTH,bgy,w-TRACK_WIDTH,1,bck_color+0x00080808);

				// beat gridline
				if(i_gridline_visible_beat && (bgi%4)==0)
				{
					if(i_gridline_visible_tick)
					{
						SetTextColor(hdc,bck_color+0x00202020);
						TextOut(hdc,6,bgy+2,buf_a,13);
						arg_gdi_paint_solid_rect(hdc,0,bgy,w,1,bck_color+0x00202020);
					}
					else
					{
						arg_gdi_paint_solid_rect(hdc,TRACK_WIDTH,bgy,w-TRACK_WIDTH,1,bck_color+0x00080808);
					}
				}

				// measure gridline
				if((bgi%(project.master_numerator*4))==0)
				{
					if(i_gridline_visible_beat)
					{
						SetTextColor(hdc,bck_color+0x00404040);
						TextOut(hdc,6,bgy+2,buf_a,13);
						arg_gdi_paint_solid_rect(hdc,0,bgy,w,1,bck_color+0x00404040);
					}
					else
					{
						arg_gdi_paint_solid_rect(hdc,TRACK_WIDTH,bgy,w-TRACK_WIDTH,1,bck_color+0x00202020);
					}
				}

				// pattern gridline
				if((bgi%(project.master_numerator*16))==0)
				{
					SetTextColor(hdc,0);
					TextOut(hdc,7,bgy+3,buf_a,13);
					SetTextColor(hdc,bck_color+0x00808080);
					TextOut(hdc,6,bgy+2,buf_a,13);
					arg_gdi_paint_solid_rect(hdc,0,bgy,w,1,bck_color+0x00808080);
				}
			}
		}

		// get piano roll screen port
		int const pr_vn=(user_pr_width-8)/user_pr_note_width;
		int const pr_no=user_kbd_note_offset-(pr_vn/2)+12;

		// paint piano-roll grid
		for(int prn=0;prn<pr_vn;prn++)
		{
			int const pr_ni=pr_no+prn;
			int const pr_nx=TRACK_WIDTH+4+prn*user_pr_note_width;

			COLORREF pr_cl=bck_color+0x00060606;

			if((pr_ni%12)==0 || pr_ni==128)
				pr_cl=bck_color+0x00101010;

			if(pr_ni==(user_kbd_note_offset+12))
				pr_cl=bck_color+0x00202020;

			if(pr_ni==user_kbd_note_offset || pr_ni==(user_kbd_note_offset+24))
				pr_cl=bck_color+0x00404040;

			if(pr_ni>=0 && pr_ni<=128)
				arg_gdi_paint_solid_rect(hdc,pr_nx,seq_area_y,1,seq_area_h,pr_cl);
		}

		// piano roll octaves
		for(prn=0;prn<pr_vn;prn++)
		{
			int const pr_ni=pr_no+prn;

			if((pr_ni%12)==0 && pr_ni>=0 && pr_ni<100)
			{
				buf_a[0]='0'+pr_ni/10;
				buf_a[1]='0'+pr_ni%10;
				paint_txt(hdc,TRACK_WIDTH+6+prn*user_pr_note_width,seq_area_y+1,buf_a,2,2);
			}
		}

		// draw back track frames
		for(t=1;t<seq_visi_t;t++)
		{
			int const tfx=TRACK_WIDTH+user_pr_width+t*TRACK_WIDTH;
			arg_gdi_paint_solid_rect(hdc,tfx-1,seq_area_y,1,seq_area_h,bck_color-0x00202020);
			arg_gdi_paint_solid_rect(hdc,tfx,seq_area_y,1,seq_area_h,bck_color+0x00202020);
		}

		// paint sequencer position separator(s)
		arg_gdi_paint_solid_rect(hdc,45,seq_area_y,1,seq_area_h,bck_color-0x00202020);
		arg_gdi_paint_solid_rect(hdc,46,seq_area_y,1,seq_area_h,bck_color+0x00202020);

		// paint piano roll separator corners
		BitBlt(hdc,TRACK_WIDTH,seq_area_y,4,16,hdc_gfx,192,572,SRCCOPY);
		BitBlt(hdc,TRACK_WIDTH+user_pr_width-4,seq_area_y,4,16,hdc_gfx,192,572,SRCCOPY);
		BitBlt(hdc,TRACK_WIDTH,seq_area_y+seq_area_h-16,4,16,hdc_gfx,200,572,SRCCOPY);
		BitBlt(hdc,TRACK_WIDTH+user_pr_width-4,seq_area_y+seq_area_h-16,4,16,hdc_gfx,200,572,SRCCOPY);

		// paint piano roll separator bars
		for(t=(seq_area_y+16);t<(seq_area_y+seq_area_h-16);t+=16)
		{
			BitBlt(hdc,TRACK_WIDTH,t,4,16,hdc_gfx,196,572,SRCCOPY);
			BitBlt(hdc,TRACK_WIDTH+user_pr_width-4,t,4,16,hdc_gfx,196,572,SRCCOPY);
		}

		// get last visible track index
		int const last_visible_track=user_trk_offset+seq_visi_t;

		// scan all sequencer events
		for(int ev=0;ev<seq_num_events;ev++)
		{
			// get sequencer event pointer
			ADX_EVENT* pe=&seq_event[ev];

			// check event parameters
			if(pe->pat==user_pat && pe->trk>=user_trk_offset && pe->trk<last_visible_track)
			{
				// get event screen coordinates
				int const e_x=TRACK_WIDTH+user_pr_width+(pe->trk-user_trk_offset)*TRACK_WIDTH;
				int const e_y=seq_cent_y+(pe->pos-pp->usr_pos)/pp->usr_ppp;
				int const e_h=gui_get_event_height(pe);
				int const e_b=e_y+e_h;

				// visible events only
				if(e_y<seq_area_b && e_b>seq_area_y)
				{
					// note event
					if(pe->typ==0)
					{
						// paint tracker event
						sprintf(buf_a,"%s %.2X %.2X %.2X %.2X",note_name[pe->da2],pe->da0,pe->da1,pe->da2,pe->da3);
						paint_event(hdc,e_x,e_y,TRACK_WIDTH,e_h,0x00808080+pe->da3*0x00010101,0x00806060,buf_a);

						// paint event in pr
						if(pe->trk==user_trk)
						{
							// get piano roll event screen x
							int const pr_ni=int(pe->da2)-pr_no;
							int const pr_ex=pr_ni*user_pr_note_width;

							// paint piano roll event
							if(pr_ni>=0 && pr_ni<pr_vn)
							{
								arg_gdi_paint_solid_rect(hdc,TRACK_WIDTH+4+pr_ex,e_y,user_pr_note_width,e_h,0x0);
								arg_gdi_paint_solid_rect(hdc,TRACK_WIDTH+5+pr_ex,e_y+1,user_pr_note_width-2,e_h-2,0x00806060+pe->da3*0x00010101);
							}
						}
					}

					// pattern event
					if(pe->typ==1)
					{
						sprintf(buf_a,"Pat %.2X %.2X %.2X %.2X",pe->da0,pe->da1,pe->da2,pe->da3);
						paint_event(hdc,e_x,e_y,TRACK_WIDTH,e_h,0x00FFFFFF,0x00608060,buf_a);

						// pattern event label
						SetTextColor(hdc,0x00FFFFFF);
						sprintf(buf_a,project.pattern[pe->da0].name);
						buf_a[15]=0;
						TextOut(hdc,e_x+3,e_y+16,buf_a,strlen(buf_a));
					}

					// jump event
					if(pe->typ==2)
						paint_event(hdc,e_x,e_y,TRACK_WIDTH,e_h,0x00FFFFFF,0x00404040,"Jmp           ");

					// midi automation event
					if(pe->typ==3)
					{
						sprintf(buf_a,"Mid %.2X %.2X %.2X %.2X",pe->da0,pe->da1,pe->da2,pe->da3);
						paint_event(hdc,e_x,e_y,TRACK_WIDTH,NR_EVENT_HEIGHT,0x00FFFFFF,0x00606080,buf_a);
					}

					// vst automation event
					if(pe->typ==4)
					{
						sprintf(buf_a,"Aut %.2X %.2X %.2X %.2X",pe->da0,pe->da1,pe->da2,pe->da3);
						paint_event(hdc,e_x,e_y,TRACK_WIDTH,NR_EVENT_HEIGHT,0x00FFFFFF,0x00404040+pe->da3/2,buf_a);
					}

					// tempo automation event
					if(pe->typ==5)
					{
						sprintf(buf_a,"Tmp %.2X %.2X %.2X %.2X",pe->da0,pe->da1,pe->da2,pe->da3);
						paint_event(hdc,e_x,e_y,TRACK_WIDTH,NR_EVENT_HEIGHT,0x00FFFFFF,0x00404040+pe->da0/2,buf_a);
					}
				}
			}
		}

		// invert user row cursor
		int const i_cursor_x=TRACK_WIDTH+user_pr_width+3+((user_trk-user_trk_offset)*TRACK_WIDTH);

		// select inverted row position
		switch(user_row)
		{
		case 0: arg_gdi_paint_invert_rect(hdc,i_cursor_x+0,seq_cent_y,18,12);	break;
		case 1:	arg_gdi_paint_invert_rect(hdc,i_cursor_x+24,seq_cent_y,6,12);	break;
		case 2:	arg_gdi_paint_invert_rect(hdc,i_cursor_x+30,seq_cent_y,6,12);	break;
		case 3:	arg_gdi_paint_invert_rect(hdc,i_cursor_x+42,seq_cent_y,6,12);	break;
		case 4:	arg_gdi_paint_invert_rect(hdc,i_cursor_x+48,seq_cent_y,6,12);	break;
		case 5:	arg_gdi_paint_invert_rect(hdc,i_cursor_x+60,seq_cent_y,6,12);	break;
		case 6:	arg_gdi_paint_invert_rect(hdc,i_cursor_x+66,seq_cent_y,6,12);	break;
		case 7:	arg_gdi_paint_invert_rect(hdc,i_cursor_x+78,seq_cent_y,6,12);	break;
		case 8:	arg_gdi_paint_invert_rect(hdc,i_cursor_x+84,seq_cent_y,6,12);	break;
		}

		// paint pattern marker(s)
		for(int m=0;m<MAX_MARKERS;m++)
		{
			// get marker pointer
			ADX_MARKER* pm=&pp->marker[m];

			// paint marker if enabled
			if(pm->flg)
			{
				sprintf(buf_a," Index %.2X",m);
				paint_marker(hdc,0x0088C8C8+(user_pressed==30)*(user_marker_drag==m)*0x00101010,2,buf_a,seq_cent_y+(pm->pos-pp->usr_pos)/pp->usr_ppp,w,-16,(user_pressed==30)*(user_marker_drag==m));
			}
		}

		// paint pattern cue-loop start marker
		gui_format_pos(pp->cue_sta,buf_a);
		paint_marker(hdc,0x00A0B0A0+(user_pressed==28)*0x00101010,0,buf_a,seq_cent_y+(pp->cue_sta-pp->usr_pos)/pp->usr_ppp,w,-16,(user_pressed==28));

		// paint pattern cue-loop end marker
		gui_format_pos(pp->cue_end,buf_a);
		paint_marker(hdc,0x00A0B0A0+(user_pressed==29)*0x00101010,3,buf_a,seq_cent_y+(pp->cue_end-pp->usr_pos)/pp->usr_ppp,w,1,(user_pressed==29));

		// paint pattern cue-stop marker
		if(pp->cue_stp>0)
		{
			gui_format_pos(pp->cue_stp,buf_a);
			paint_marker(hdc,0x0060C0A0+(user_pressed==6)*0x00101010,1,buf_a,seq_cent_y+(pp->cue_stp-pp->usr_pos)/pp->usr_ppp,w,-16,(user_pressed==6));
		}

		// get block length
		int const user_block_pos_len=user_block_pos_end-user_block_pos_sta;
		int const user_block_trk_len=user_block_trk_end-user_block_trk_sta;

		// draw block mark
		if(user_block_trk_len>0 && user_block_pos_len>0)
		{
			// get block mark vertical coordinates
			int const src_bm_y=seq_cent_y+(user_block_pos_sta-pp->usr_pos)/pp->usr_ppp;
			int const scr_bm_h=user_block_pos_len/pp->usr_ppp;

			// start clip
			paint_block_shade(hdc,TRACK_WIDTH+user_pr_width+(user_block_trk_sta-user_trk_offset)*TRACK_WIDTH,src_bm_y,user_block_trk_len*TRACK_WIDTH,scr_bm_h,TRACK_WIDTH+user_pr_width,seq_area_y,w,h);

			// invert block mark in piano roll view
			if(user_trk>=user_block_trk_sta && user_trk<(user_block_trk_sta+user_block_trk_len))
				paint_block_shade(hdc,TRACK_WIDTH+4,src_bm_y,user_pr_width-8,scr_bm_h,0,seq_area_y,w,h);
		}

		// alphablend edit quantize size shade
		bf.SourceConstantAlpha=24;
		AlphaBlend(hdc,0,seq_cent_y,w,i_quantize/pp->usr_ppp,hdc_gfx,240,0,16,16,bf);

		// alphablend edit cursor position
		bf.SourceConstantAlpha=96;
		AlphaBlend(hdc,0,seq_cent_y,w,1,hdc_gfx,240,0,16,16,bf);

		// invert player transport rect
		if(master_time_info.flags & kVstTransportPlaying)
		{
			// get transport gridline coordinate
			int const i_ty=seq_cent_y+(i_master_play_pos-pp->usr_pos)/pp->usr_ppp;

			// paint play-gridline
			arg_gdi_paint_invert_rect(hdc,0,i_ty,w,2);

			// update last gridline coordinate 
			master_transport_last_y=i_ty;
		}

		// end clip
		arg_gdi_end_clip_rgn(hdc);
	}

	// routing page
	if(user_page==1 && h>232)
	{
		// routing view client area
		int const rout_area_y=232;
		int const rout_area_h=h-rout_area_y;

		// rout clip region
		arg_gdi_set_clip_rgn(hdc,0,rout_area_y,w,rout_area_h);

		// paint back
		arg_gdi_paint_solid_rect(hdc,0,rout_area_y,w,rout_area_h,bck_color);

		// paint rout horizontal grid
		for(int xg=0;xg<w;xg+=64)
			arg_gdi_paint_solid_rect(hdc,xg-(user_rout_offset_x%64),rout_area_y,1,rout_area_h,bck_color+0x00080808);

		// paint rout vertical grid
		for(int yg=0;yg<rout_area_h;yg+=64)
			arg_gdi_paint_solid_rect(hdc,0,rout_area_y+yg-(user_rout_offset_y%64),w,1,bck_color+0x00080808);

		// paint rout routing center offset-axis
		arg_gdi_paint_solid_rect(hdc,-user_rout_offset_x,rout_area_y,1,rout_area_h,bck_color+0x00202020);
		arg_gdi_paint_solid_rect(hdc,0,rout_area_y-user_rout_offset_y,w,1,bck_color+0x00202020);
		arg_gdi_paint_solid_rect(hdc,-user_rout_offset_x,rout_area_y-user_rout_offset_y,1,1,bck_color+0x00808080);

		// get master input module screen coordinates
		int const m_i_x=master_i_x-user_rout_offset_x;
		int const m_i_y=rout_area_y+master_i_y-user_rout_offset_y;

		// get master output module screen coordinates
		int const m_o_x=master_o_x-user_rout_offset_x;
		int const m_o_y=rout_area_y+master_o_y-user_rout_offset_y;

		// paint master input box
		BitBlt(hdc,m_i_x,m_i_y,256,16,hdc_gfx,64,16+(user_pressed==37)*16,SRCCOPY);

		// paint master input connection panel
		for(int i=0;i<NUM_DSP_INPUTS;i++)
			BitBlt(hdc,m_i_x+i*8,m_i_y+16,8,16,hdc_gfx,112+(cfg.asio_input_pin[i]>=asio_num_inputs)*16,0,SRCCOPY);

		// paint master input selected pin
		BitBlt(hdc,m_i_x+user_input_pin*8,m_i_y+16,8,8,hdc_gfx,104,0,SRCCOPY);

		// paint master input pin array
		paint_pin_array(hdc,m_i_x,m_i_y+28,NUM_DSP_INPUTS,master_input_pin,m_o_x,m_o_y,rout_area_y);

		// paint master output box
		BitBlt(hdc,m_o_x,m_o_y+8,256,16,hdc_gfx,64,48+(user_pressed==38)*16,SRCCOPY);

		// paint master output connection panel
		for(int o=0;o<NUM_DSP_OUTPUTS;o++)
			BitBlt(hdc,m_o_x+o*8,m_o_y,8,8,hdc_gfx,120+(cfg.asio_output_pin[o]>=asio_num_outputs)*8,0,SRCCOPY);

		// paint master output selected pin
		BitBlt(hdc,m_o_x+user_output_pin*8,m_o_y,8,8,hdc_gfx,104,0,SRCCOPY);

		// text blck color
		SetTextColor(hdc,0);

		// paint all instance boxes
		for(i=0;i<MAX_INSTANCES;i++)
		{
			// get instance pointer
			ADX_INSTANCE* pi=&instance[i];

			// check if instance is loaded
			if(pi->peffect!=NULL)
			{
				// get num in and out pins (audio + midi)
				int const i_ni=pi->peffect->numInputs+1;
				int const i_no=pi->peffect->numOutputs+1;

				// get instance module screen coordinates and box width
				int const i_x=pi->x-user_rout_offset_x;
				int const i_y=rout_area_y+pi->y-user_rout_offset_y;
				int const i_w=arg_tool_clipped_assign(max(i_ni,i_no)*8,128,MAX_SIGNED_INT);

				// blit instance box
				BitBlt(hdc,i_x,i_y,i_w-2,48,hdc_gfx,0,672,SRCCOPY);
				BitBlt(hdc,i_x+i_w-2,i_y,2,48,hdc_gfx,262,672,SRCCOPY);

				// get instance current program label
				pi->peffect->dispatcher(pi->peffect,effGetProgramName,0,0,buf_a,0.0f);

				// paint instance labels
				TextOut(hdc,i_x+36,i_y+12,pi->alias,min(strlen(pi->alias),(i_w-40)/6));
				TextOut(hdc,i_x+4,i_y+28,buf_a,min(strlen(buf_a),(i_w-8)/6));

				// paint machine audio inputs
				for(int in=0;in<pi->peffect->numInputs;in++)
					BitBlt(hdc,i_x+in*8,i_y,8,8,hdc_gfx,64,0,SRCCOPY);

				// paint machine midi input pin
				BitBlt(hdc,i_x+in*8,i_y,8,8,hdc_gfx,80,0,SRCCOPY);

				// paint machine audio outputs
				for(int ou=0;ou<pi->peffect->numOutputs;ou++)
					BitBlt(hdc,i_x+ou*8,i_y+40,8,8,hdc_gfx,72,0,SRCCOPY);

				// paint machine midi output pin
				BitBlt(hdc,i_x+ou*8,i_y+40,8,8,hdc_gfx,80,0,SRCCOPY);

				// instance list mute
				if(pi->process_mute)
					BitBlt(hdc,i_x,i_y+8,16,16,hdc_gfx,192,0,SRCCOPY);

				// instance list thru (bypass)
				if(pi->process_thru)
					BitBlt(hdc,i_x+16,i_y+8,16,16,hdc_gfx,208,0,SRCCOPY);

				// is current instance
				if(user_instance==i)
				{
					// alphablend selected machine
					bf.SourceConstantAlpha=64;
					AlphaBlend(hdc,i_x,i_y,i_w,48,hdc_gfx,240,0,16,16,bf);
				}

				// paint instance audio output pin(s) array
				paint_pin_array(hdc,i_x,i_y+44,pi->peffect->numOutputs,pi->pout_pin,m_o_x,m_o_y,rout_area_y);

				// paint midi-out link wires
				for(int w=0;w<pi->mout_pin.num_wires;w++)
				{
					// midi-out link source-destination screen coordinates
					int const mol_x1=i_x+ou*8;
					int const mol_y1=i_y+40;

					// get midi-out wire
					ADX_WIRE* pw=&pi->mout_pin.pwire[w];

					// get midi-out link destination instance
					ADX_INSTANCE* pi_mol=&instance[pw->instance_index];

					// check that destination instance is loaded
					if(pi_mol->peffect!=NULL)
					{
						int const mol_x2=pi_mol->x-user_rout_offset_x+4+pi_mol->peffect->numInputs*8;
						int const mol_y2=rout_area_y+pi_mol->y-user_rout_offset_y+4;

						// paint midi-out link wire
						paint_wire(hdc,mol_x1+4,mol_y1+4,mol_x2,mol_y2,0x0080FFFF);
					}
				}
			}
		}

		// paint dragging midi-out link wire
		if(user_pressed==25)
		{
			// get instance wire source
			ADX_INSTANCE* pi=&instance[user_dragging_rout_instance_index];

			// get instance module screen coordinates
			int const i_x=pi->x-user_rout_offset_x;
			int const i_y=rout_area_y+pi->y-user_rout_offset_y;

			// paint wire
			paint_wire(hdc,i_x+4+pi->peffect->numOutputs*8,i_y+44,user_lxm,user_lym,0x00FFFFFF);
		}

		// paint dragging instance audio wire
		if(user_pressed==36)
		{
			// instance wire
			if(user_dragging_rout_instance_index<MAX_INSTANCES)
			{
				// get instance wire source
				ADX_INSTANCE* pi=&instance[user_dragging_rout_instance_index];

				// get instance module screen coordinates
				int const i_x=pi->x-user_rout_offset_x;
				int const i_y=rout_area_y+pi->y-user_rout_offset_y;

				// paint wire
				paint_wire(hdc,i_x+4+user_dragging_rout_pin_index*8,i_y+44,user_lxm,user_lym,0x00FFFFFF);
			}

			// paint dragging master-input wire
			if(user_dragging_rout_instance_index==INPUT_INSTANCE)
				paint_wire(hdc,m_i_x+4+user_dragging_rout_pin_index*8,m_i_y+28,user_lxm,user_lym,0x00FFFFFF);
		}

		// paint wire gain bar
		if(user_pressed==44)
		{
			// fill ab struct
			bf.SourceConstantAlpha=128;
			AlphaBlend(hdc,user_pressed_wire_x-18,user_pressed_wire_y-66,37,133,hdc_gfx,576,0,37,133,bf);
			
			// get inversed gain
			float const inv_gain=1.0-user_pressed_wire->value;

			// integer gain offset
			int const i_gain=int(inv_gain*112.0f);

			// slider position
			int const s_xp=user_pressed_wire_x-15;
			int const s_yp=(user_pressed_wire_y-63)+i_gain;

			// blit gain slider
			BitBlt(hdc,s_xp,s_yp,32,16,hdc_gfx,613,0,SRCCOPY);
		
			// format gain text
			sprintf(buf_a,"%.3d%%",int(user_pressed_wire->value*100.0f));
			paint_txt(hdc,s_xp+3,s_yp+4,buf_a,4,3);
		}

		// end clip
		arg_gdi_end_clip_rgn(hdc);
	}

	// set default font
	SelectObject(hdc,(HFONT)hfont_default);
}
