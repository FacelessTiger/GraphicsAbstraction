#      <              Ά    +     ,     1   
  SPV_EXT_descriptor_indexing      GLSL.std.450              	       main                             Assets/shaders/trianglePixel.hlsl            #include "Cobra.hlsl"

struct PushConstant
{
	float4x4 projection;
	float3 cameraPos;
	uint vertices;
	uint modelMatrices;
};
PushConstant(PushConstant, pushConstants);

struct VertexInput
{
	float4 position: SV_Position;
	float3 worldPosition: POSITION0;
	float3 normal: NORMAL0;
	float3 color: COLOR0;
	float2 uv: TEXCOORD0;
};

float4 main(VertexInput input): SV_Target
{
	float3 lightPos = float3(0.0f, 5.0f, 5.0f);

	float3 lightColor = float3(1.0f, 1.0f, 1.0f);
	float ambientStength = 0.1;
	float specularStrength = 0.5;

	float3 norm = normalize(input.normal);
	float3 lightDir = normalize(lightPos - input.worldPosition);
	float3 viewDir = normalize(pushConstants.cameraPos - input.worldPosition);
	float3 reflectDir = reflect(-lightDir, norm);

	float3 ambient = ambientStength * lightColor;

	float diff = max(dot(norm, lightDir), 0.0);
	float3 diffuse = diff * lightColor;

	float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
	float3 specular = specularStrength * spec * lightColor;

	return float4((ambient + diffuse + specular) * input.color, 1.0f);
}   
    type.PushConstant.PushConstant           projection          cameraPos           vertices            modelMatrices     	   pushConstants        in.var.POSITION0         in.var.NORMAL0       in.var.COLOR0        out.var.SV_Target        main    G            G           G           G            H         #       H               H            H        #   @   H        #   L   H        #   P   G          
       +  
          +  
         @     
      ,                 +  
        ?,                 +  
      ΝΜΜ=+  
         ?            +           +  
         B                  
                                        	                                   !              	      ;     	   	   ;           ;           ;           ;           ,                 6               ψ      =     !      =     "      =     #                      $      E   "           '        %      !                   &      E   %              A     '   	              +   =     (   '           5        )   (   !                   *      E   )                    +   &                    ,      G   +   $        $        
   -   $   &        $        
   .      P   -           %           /      .        '        
   0   *   ,        '        
   1      P   0           '        
   2         1           (   %     
   3      2        (   ,        4      3        *           5      /        *   #        6   5   4        *   /        7   6   #        *      Q  
   8   7       Q  
   9   7      Q  
   :   7      P     ;   8   9   :      = >     ;        +      ύ  8  