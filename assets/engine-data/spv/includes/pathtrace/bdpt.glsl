#ifndef pt_bdpt_glsl
#define pt_bdpt_glsl

void addSample(image2D image, float imageX, float imageY, vec3 L) {

	float dImageX = imageX - 0.5f;
    float dImageY = imageY - 0.5f;
    // calculate the pixel range covered by filter center at sample
    float xWidth = float(gl_LaunchSizeEXT.x);//mFilter->getXWidth();
    float yWidth = float(gl_LaunchSizeEXT.y);//mFilter->getYWidth();
    int x0 = ceil(dImageX - xWidth);
    int x1 = floor(dImageX + xWidth);
    int y0 = ceil(dImageY - yWidth);
    int y1 = floor(dImageY + yWidth);
    //x0 = max(x0, mXStart);
    //x1 = min(x1, mXStart + mXCount - 1);
    //y0 = max(y0, mYStart);
    //y1 = min(y1, mYStart + mYCount - 1);
    
    for(int y = y0; y <= y1; ++y) {
        float fy = abs(gl_LaunchSizeEXT.x * (y - dImageY) / yWidth);
        int iy = min(floor(fy), gl_LaunchSizeEXT.x - 1);
        for(int x = x0; x <= x1; ++x) {
            float fx = fabs(gl_LaunchSizeEXT.x * (x - dImageX) / xWidth);
            int ix = min(floor(fx), gl_LaunchSizeEXT.x - 1);
            float weight = 1.0;
            int index = y * mXRes + x;

            vec3 color = weight * L;

            if(frame > 0) {
		    	float a = 1.0f / float(frame);
		    	vec3 old_color = imageLoad(image, pixel).xyz;
		    	color = mix(old_color, color, a);
		    }

            imageStore(image, ivec2(x,y), vec4(color, 1.f));
        }
    }

}

#endif
