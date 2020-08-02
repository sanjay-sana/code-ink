/**
 * @author mrdoob / http://mrdoob.com/
 * @author alteredq / http://alteredqualia.com/
 */
THREE.Raycaster = function ( origin, direction) {
	this.ray = new THREE.Ray( origin, direction );
};

var calculateIntersection = function ( object, raycaster, intersects, recursive ) {
	object.raycast( raycaster, intersects );
	if ( recursive === true ) {
		var children = object.children;
		for ( var i = 0, l = children.length; i < l; i ++ ) {
			calculateIntersection( children[ i ], raycaster, intersects, true );
		}
	}
};


THREE.Raycaster.prototype = {
	
	constructor: THREE.Raycaster,
	precision: 0.0001,
		
	set: function ( origin, direction ) {
		this.ray.set( origin, direction );
	},

	calculateIntersections: function ( objects, recursive ) {
		//var intersects = [];
		var ObjectOrderArray = [];
		if ( objects instanceof Array === false ) {
			return ObjectOrderArray;
		}

		for ( var i = 0; i < objects.length; i ++ )
				calculateIntersection( objects[ i ], this, ObjectOrderArray, recursive );

		ObjectOrderArray.sort(function ( a, b ) {
	
			return a.distance - b.distance;
		});
		return ObjectOrderArray;
	}
};

THREE.RaytracingRenderer = function ( parameters ) {

	// console.log( 'THREE.RaytracingRenderer', THREE.REVISION );

	parameters = parameters || {};

	var scope = this;

	var canvas = document.createElement( 'canvas' );
	var context = canvas.getContext( '2d', {
		alpha: parameters.alpha === true
	} );

	//var maxRecursionDepth = 3;

	var canvasWidth, canvasHeight;
	var canvasWidthHalf, canvasHeightHalf;

	var clearColor = new THREE.Color( 0x000000 );

	var origin = new THREE.Vector3();
	var direction = new THREE.Vector3();

	var cameraPosition = new THREE.Vector3();

	var raycaster = new THREE.Raycaster( origin, direction );
	var raycasterLight = new THREE.Raycaster();

	var perspective;
	var modelViewMatrix = new THREE.Matrix4();
	var cameraNormalMatrix = new THREE.Matrix3();

	var objects;
	var lights = [];
	var cache = {};

	var animationFrameId = null;

	this.domElement = canvas;

	this.autoClear = true;

	this.setClearColor = function ( color, alpha ) {

		clearColor.set( color );

	};

	this.setPixelRatio = function () {};

	this.setSize = function ( width, height ) {

		canvas.width = width;
		canvas.height = height;

		canvasWidth = canvas.width;
		canvasHeight = canvas.height;

		context.fillStyle = 'white';
	};

	this.setSize( canvas.width, canvas.height );

	var executeRay = ( function () {

		var diffuseColor = new THREE.Color();
		var specularColor = new THREE.Color();
		var lightColor = new THREE.Color();
		//var schlick = new THREE.Color();
		var approximation = new THREE.Color();

		var lightContribution = new THREE.Color();

		var eyeVector = new THREE.Vector3();
		var lightVector = new THREE.Vector3();
		var normalVector = new THREE.Vector3();
		var halfVector = new THREE.Vector3();

		var localPoint = new THREE.Vector3();
		

		var tmpVector = new THREE.Vector3();

		var tmpColor = [];

		for ( var i = 0; i < 3; i ++ ) {

			tmpColor[ i ] = new THREE.Color();

		}

		return function ( rayOrigin, rayDirection, outputColor, recursionDepth ) {

			var ray = raycaster.ray;

			ray.origin = rayOrigin;
			ray.direction = rayDirection;

			//

			var rayLight = raycasterLight.ray;

			//

			// outputColor.setRGB( 0, 0, 0 );
			outputColor.r = 0
			outputColor.g = 0
			outputColor.b = 0
			//

			var intersections = raycaster.calculateIntersections( objects, true );

			// ray didn't find anything
			// (here should come setting of background color?)

			if ( intersections.length === 0 )
				return;


			// ray hit

			var intersection = intersections[ 0 ];

			var point = intersection.point;
			var object = intersection.object;
			var material = object.material;
			var face = intersection.face;

			var vertices = object.geometry.vertices;

			var _object = cache[ object.id ];

            localPoint.x = point.x;
            localPoint.y = point.y;
            localPoint.z = point.z;
            localPoint.applyMatrix4( _object.inverseMatrix );
            
            eyeVector.x = raycaster.ray.origin.x - point.x;
		    eyeVector.y = raycaster.ray.origin.y - point.y;
		    eyeVector.z = raycaster.ray.origin.z - point.z;
            
            eyeVector.normalize();
			//step1: resolve pixel diffuse color

			if ( material instanceof THREE.MeshPhongMaterial ) {

                var gammaFactor = 2.0;
                diffuseColor.r = Math.pow( material.color.r, gammaFactor );
		        diffuseColor.g = Math.pow( material.color.g, gammaFactor );
		        diffuseColor.b = Math.pow( material.color.b, gammaFactor );

			} else {

				diffuseColor.r = 1;
		        diffuseColor.g = 1;
		        diffuseColor.b = 1;
			}

			if ( material.vertexColors === THREE.FaceColors ) {
                diffuseColor.r *= face.color.r;
		        diffuseColor.g *= face.color.g;
		        diffuseColor.b *= face.color.b;
			}

			// compute light shading

			rayLight.origin.copy( point );

			if ( material instanceof THREE.MeshPhongMaterial ) {
				var normalized = false;

				for ( var i = 0, l = lights.length; i < l; i ++ ) {

					var light = lights[ i ];

					lightColor.copyGammaToLinear( light.color );

					lightVector.setFromMatrixPosition( light.matrixWorld );
					lightVector.x -= point.x;
					lightVector.y -= point.y;
					lightVector.z -= point.z;

					rayLight.direction.copy( lightVector ).normalize();

					var intersections = raycasterLight.calculateIntersections( objects, true );

					// point in shadow

					if ( intersections.length > 0 ) continue;

					// point lit

					if ( normalized === false ) {

						// the same normal can be reused for all lights
						// (should be possible to cache even more)

						Normalization( normalVector, localPoint, material.shading, face, vertices );
						normalVector.applyMatrix3( _object.normalMatrix ).normalize();

						normalized = true;

					}

					// compute attenuation

					var attenuation = 1.0;

					if ( light.physicalAttenuation === true ) {

						attenuation = lightVector.length();
						attenuation = 1.0 / ( attenuation * attenuation );

					}

					lightVector.normalize();

					// compute diffuse

					var diffuseIntensity = Math.max( normalVector.dot( lightVector ), 0 ) * light.intensity;

					lightContribution.copy( diffuseColor );
					lightContribution.r *= lightColor.r * diffuseIntensity * attenuation;
					lightContribution.g *= lightColor.g * diffuseIntensity * attenuation;
					lightContribution.b *= lightColor.b * diffuseIntensity * attenuation;

					outputColor.r += lightContribution.r;
					outputColor.g += lightContribution.g;
					outputColor.b += lightContribution.b;

					// compute specular

					if ( material instanceof THREE.MeshPhongMaterial ) {

						halfVector.addVectors( lightVector, eyeVector ).normalize();

						var dotNormalHalf = Math.max( normalVector.dot( halfVector ), 0.0 );
						var specularIntensity = Math.max( Math.pow( dotNormalHalf, material.shininess ), 0.0 ) * diffuseIntensity;

						var specularNormalization = ( material.shininess + 2.0 ) / 8.0;

						specularColor.copyGammaToLinear( material.specular );

						var alpha = Math.pow( Math.max( 1.0 - lightVector.dot( halfVector ), 0.0 ), 5.0 );

						approximation.r = specularColor.r + ( 1.0 - specularColor.r ) * Math.pow( Math.max( 1.0 - lightVector.dot( halfVector ), 0.0 ), 5.0 );
						approximation.g = specularColor.g + ( 1.0 - specularColor.g ) * Math.pow( Math.max( 1.0 - lightVector.dot( halfVector ), 0.0 ), 5.0 );
						approximation.b = specularColor.b + ( 1.0 - specularColor.b ) * Math.pow( Math.max( 1.0 - lightVector.dot( halfVector ), 0.0 ), 5.0 );

						lightContribution.copy( approximation );
						lightContribution.r *= lightColor.r * specularNormalization * specularIntensity * attenuation;
						lightContribution.g *= lightColor.g * specularNormalization * specularIntensity * attenuation;
						lightContribution.b *= lightColor.b * specularNormalization * specularIntensity * attenuation;
						outputColor.r += lightContribution.r;
						outputColor.g += lightContribution.g;
						outputColor.b += lightContribution.b;

					}

				}

			}

			//step3: reflection / refraction
			var reflectionVector = new THREE.Vector3();

			if ( material.mirror && material.reflectivity > 0 && recursionDepth < 3) 	//reflect
			{
					reflectionVector.copy( rayDirection );						
					var v1 = new THREE.Vector3();
					reflectionVector.sub(v1.copy(normalVector).multiplyScalar(2*(reflectionVector.dot(normalVector))));
					
					var theta = Math.max( eyeVector.dot( normalVector ), 0.0 );
					var fresnel = material.reflectivity + ( 1.0 - material.reflectivity ) * Math.pow( ( 1.0 - theta ), 5.0 );
					var weight = fresnel;
					var zColor = tmpColor[ recursionDepth ];
					executeRay( point, reflectionVector, zColor, recursionDepth + 1 );
					if ( material.specular !== undefined ) {
						zColor.multiply( material.specular );
					}
					zColor.multiplyScalar( weight );
					outputColor.multiplyScalar( 1 - weight );
					outputColor.add( zColor );
			} else if ( material.glass && material.reflectivity > 0 && recursionDepth < 3)	//refract
			{
				var eta = material.refractionRatio;
				var dotNI = rayDirection.dot( normalVector )
				var k = 1.0 - eta * eta * ( 1.0 - dotNI * dotNI );
	
				if ( k < 0.0 ) 
				{
					reflectionVector.set( 0, 0, 0 );
				}else
				{
					reflectionVector.copy( rayDirection );
					reflectionVector.multiplyScalar( eta );//Multiplies this vector by scalar s.
					var alpha;
					if(dotNI >= 0.0)
					{
						alpha = eta * dotNI - Math.sqrt( k );	
					}else
					{
						alpha = eta * dotNI + Math.sqrt( k );
					}
					tmpVector.copy( normalVector );
					tmpVector.multiplyScalar( alpha );//Multiplies this vector by scalar s.
					reflectionVector.sub( tmpVector );//Subtracts v from this vector.
				}
				var fresnel = material.reflectivity + ( 1.0 - material.reflectivity ) * Math.pow( ( 1.0 - Math.max( eyeVector.dot( normalVector ), 0.0 ) ), 5.0 );
				var weight = fresnel;
				var zColor = tmpColor[ recursionDepth ];
				executeRay( point, reflectionVector, zColor, recursionDepth + 1 );
				if ( material.specular !== undefined ) {
					zColor.multiply( material.specular );
				}
				zColor.multiplyScalar( weight );
				outputColor.multiplyScalar( 1 - weight );
				outputColor.add( zColor );			
			}
		};
	}() );

	var Normalization = ( function () {
		var tmpVector = {};
		for(var i=0; i<3; i++)
			tmpVector[i] = new THREE.Vector3();

		return function ( outputVector, point, shading, face, vertices ) {
			if ( shading == THREE.FlatShading ) {

				outputVector.copy( face.normal );

			} else if ( shading == THREE.SmoothShading ) {

				// compute barycentric coordinates
				var vA = vertices[ face.a ];
				var vB = vertices[ face.b ];
				var vC = vertices[ face.c ];

				tmpVector[0].x = vB.x - vA.x;
				tmpVector[0].y = vB.y - vA.y;
				tmpVector[0].z = vB.z - vA.z;
				var vec1 = tmpVector[0];
				
				tmpVector[1].x = vC.x - vA.x;
				tmpVector[1].y = vC.y - vA.y;
				tmpVector[1].z = vC.z - vA.z;
				var vec2 = tmpVector[1];

				tmpVector[2].x = vec1.y * vec2.z - vec1.z * vec2.y;
				tmpVector[2].y = vec1.z * vec2.x - vec1.x * vec2.z;
				tmpVector[2].z = vec1.x * vec2.y - vec1.y * vec2.x;
				
				var areaABC = face.normal.dot( tmpVector[2] );

				tmpVector[0].x = vB.x - point.x;
				tmpVector[0].y = vB.y - point.y;
				tmpVector[0].z = vB.z - point.z;
				vec1 = tmpVector[0];

				tmpVector[1].x = vC.x - point.x;
				tmpVector[1].y = vC.y - point.y;
				tmpVector[1].z = vC.z - point.z;
				vec2 = tmpVector[1];

				tmpVector[2].x = vec1.y * vec2.z - vec1.z * vec2.y;
				tmpVector[2].y = vec1.z * vec2.x - vec1.x * vec2.z;
				tmpVector[2].z = vec1.x * vec2.y - vec1.y * vec2.x;

				var areaPBC = face.normal.dot( tmpVector[2] );
				var a = areaPBC / areaABC;
				
				tmpVector[0].x = vC.x - point.x;
				tmpVector[0].y = vC.y - point.y;
				tmpVector[0].z = vC.z - point.z;
				vec1 = tmpVector[0];

				tmpVector[1].x = vA.x - point.x;
				tmpVector[1].y = vA.y - point.y;
				tmpVector[1].z = vA.z - point.z;
				vec2 = tmpVector[1];

				tmpVector[2].x = vec1.y * vec2.z - vec1.z * vec2.y;
				tmpVector[2].y = vec1.z * vec2.x - vec1.x * vec2.z;
				tmpVector[2].z = vec1.x * vec2.y - vec1.y * vec2.x;

				var areaPCA = face.normal.dot( tmpVector[2] );

				var b = areaPCA / areaABC;

				var c = 1.0 - a - b;

				// compute interpolated vertex normal

				tmpVector[0].copy( face.vertexNormals[ 0 ] );
				tmpVector[0].x *= a;
				tmpVector[0].y *= a;
				tmpVector[0].z *= a;

				tmpVector[1].copy( face.vertexNormals[ 1 ] );
				tmpVector[1].x *= b;
				tmpVector[1].y *= b;
				tmpVector[1].z *= b;

				tmpVector[2].copy( face.vertexNormals[ 2 ] );
				tmpVector[2].x *= c;
				tmpVector[2].y *= c;
				tmpVector[2].z *= c;

				outputVector.x = tmpVector[0].x + tmpVector[1].x + tmpVector[2].x;
				outputVector.y = tmpVector[0].y + tmpVector[1].y + tmpVector[2].y;
				outputVector.z = tmpVector[0].z + tmpVector[1].z + tmpVector[2].z;
			}

		};

	}() );


	var renderBlock = ( function () {

		var blockSize = 64;

		var canvasBlock = document.createElement( 'canvas' );
		canvasBlock.width = blockSize;
		canvasBlock.height = blockSize;

		var contextBlock = canvasBlock.getContext( '2d', {

			alpha: parameters.alpha === true

		} );

		var imagedata = contextBlock.getImageData( 0, 0, blockSize, blockSize );
		var data = imagedata.data;

		var pixelColor = new THREE.Color();

		return function ( blockX, blockY ) {

			var index = 0;

			for ( var y = 0; y < blockSize; y ++ ) {

				for ( var x = 0; x < blockSize; x ++, index += 4 ) {

					// spawn primary ray at pixel position

					origin.copy( cameraPosition );

					direction.set( x + blockX -  Math.floor( canvasWidth / 2 ), - ( y + blockY -  Math.floor( canvasHeight / 2 ) ), - perspective );
					direction.applyMatrix3( cameraNormalMatrix ).normalize();

					executeRay( origin, direction, pixelColor, 0 );

					// convert from linear to gamma
					data[ index ]     = Math.sqrt( pixelColor.r ) * 255;
					data[ index + 1 ] = Math.sqrt( pixelColor.g ) * 255;
					data[ index + 2 ] = Math.sqrt( pixelColor.b ) * 255;
				}
			}
			
			//anti-aliasing
			index =0 ;
			for ( var y = 0; y < blockSize; y ++ ) {
				
				for ( var x = 0; x < blockSize; x ++ ) {
					var red = 0;
					var green = 0;
					var blue = 0;
					var windowSize = 3;
					for(var i=0; i< windowSize; i++){
						for(var j=0; j<windowSize; j++){
							red   += data[blockSize*y*4 + 4*x +  i * 4 + 4*j*blockSize];
							green += data[1 + blockSize*y*4 + 4*x +  i * 4 + 4*j*blockSize];
							blue  += data[2 + blockSize*y*4 + 4*x +  i * 4 + 4*j*blockSize];
						}
					}
						
					data[index]     = red/9;
					data[index + 1] = green/9;
					data[index + 2] = blue/9;
					index += 4;
				}
			}

			context.putImageData( imagedata, blockX, blockY );

			blockX += (blockSize-4);

			if ( blockX >= canvasWidth ) {
				blockX = 0;
				blockY += (blockSize-4);

				if ( blockY >= canvasHeight ) {
					return;
				}
			}

			context.fillRect( blockX, blockY, blockSize, blockSize );

			animationFrameId = requestAnimationFrame( function () {
				renderBlock( blockX, blockY );
			} );
		};
	}() );

	this.render = function ( scene, camera ) {

		cancelAnimationFrame( animationFrameId );

		// update scene graph

		if ( scene.autoUpdate === true ) scene.updateMatrixWorld();

		// update camera matrices

		if ( camera.parent === undefined ) camera.updateMatrixWorld();

		camera.matrixWorldInverse.getInverse( camera.matrixWorld );
		cameraPosition.setFromMatrixPosition( camera.matrixWorld );

		cameraNormalMatrix.getNormalMatrix( camera.matrixWorld );
		origin.copy( cameraPosition );

		perspective = 0.5 / Math.tan( THREE.Math.degToRad( camera.fov * 0.5 ) ) * canvasHeight;

		objects = scene.children;

		// collect lights and set up object matrices

		lights.length = 0;

		scene.traverse( function ( object ) {

			if ( object instanceof THREE.Light ) {

				lights.push( object );

			}

			if ( cache[ object.id ] === undefined ) {

				cache[ object.id ] = {
					normalMatrix: new THREE.Matrix3(),
					inverseMatrix: new THREE.Matrix4()
				};

			}

			modelViewMatrix.multiplyMatrices( camera.matrixWorldInverse, object.matrixWorld )

			var _object = cache[ object.id ];

			_object.normalMatrix.getNormalMatrix( modelViewMatrix );
			_object.inverseMatrix.getInverse( object.matrixWorld );

		} );

		renderBlock( 0, 0 );

	};

};

THREE.EventDispatcher.prototype.apply(THREE.RaytracingRenderer.prototype);
