#version 430

out vec4 color;

in vec2 uv;

uniform sampler2D tex;

uniform vec2 scale = vec2(500, 700);


struct BorderStyle{
	float roundedCornerSize;
};

void border(BorderStyle style);

void main(){
	color = texture(tex, uv);
	//
	//BorderStyle test;
	//test.roundedCornerSize = .121;
	//
	//border(test);
}

void border(BorderStyle style){
	float yFactor = scale.y / scale.x;
	vec2 cUv = uv * vec2(1, yFactor);//corrected uvs

	const vec2 posA = vec2(1 - style.roundedCornerSize, 1 - style.roundedCornerSize);
	if(cUv.x > posA.x && cUv.y > posA.y){
		if(distance(cUv, posA) > style.roundedCornerSize){
			color.a = 0;
		}
	}
	const vec2 posB = vec2(style.roundedCornerSize, 1 - style.roundedCornerSize);
	if(cUv.x < posB.x && cUv.y > posB.y){
		if(distance(cUv, posB) > style.roundedCornerSize){
			color.a = 0;
		}
	}
	const vec2 posC = vec2(1 - style.roundedCornerSize, style.roundedCornerSize);
	if(cUv.x > posC.x && cUv.y < posC.y){
		if(distance(cUv, posC) > style.roundedCornerSize){
			color.a = 0;
		}
	}
	const vec2 posD = vec2(style.roundedCornerSize, style.roundedCornerSize);
	if(cUv.x < posD.x && cUv.y < posD.y){
		if(distance(cUv, posD) > style.roundedCornerSize){
			color.a = 0;
		}
	}
}