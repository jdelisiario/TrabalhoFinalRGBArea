#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;
using namespace cv;

int thresh = 100;
int max_thresh = 255;
RNG rng(12345);
vector<Vec4i> hierarchy;


int main(int, char** argv)
{
    Mat src = imread("bolor5.jpg");

    //Verificação se a imagem está vazia
    if (!src.data)
        return -1;

    // Exibe a imagem original
    imshow("Original", src);

    //Converte a imagem original para escala de cinza
    Mat destino;
    cvtColor(src, destino, CV_RGB2GRAY);
    imshow("Escala de cinza", destino);

    //Realiza o corte do fundo, para destacar apenas objetos relevantes
    threshold(destino, destino, 120, 250, 0);
    imshow("Após o threshold", destino);

    Mat kernel1 = getStructuringElement( MORPH_ELLIPSE, Size(14, 14), Point(9, 9) );

    dilate(destino, destino, kernel1);

    imshow("Após dilatação", destino);

    Mat dist;

    distanceTransform(destino, dist, CV_DIST_L2, 3);
    normalize(dist, dist, 0, 1., NORM_MINMAX);
    imshow("Transformada da distância", dist);

    //Novo threshold feito sobre a transformada para separar os grãos
    threshold(dist, destino, .01, 1., CV_THRESH_BINARY);
  //  imshow("Novo threshold após a transformada de distância", destino);

    dilate(destino, destino, kernel1);
    erode(destino, destino, kernel1);

    erode(destino, destino, kernel1);

    erode(destino, destino, kernel1);
    dilate(destino, destino, kernel1);

    //Novo threshold feito sobre a transformada para separar os grãos
    threshold(dist, dist, .01, 1., CV_THRESH_BINARY);
    imshow("Novo threshold após abertura, erosão e fechamento", dist);

    //Para fazer a detecção de bordas é necessário converter a imagem para CV_8U
    Mat dist_8u;
    dist.convertTo(dist_8u, CV_8U);

    //Os contornos serão armazenados em vetor de pontos
    vector<vector<Point> > contours;

    //Encontra os contornos em uma imagem binária
    //CV_RETR_EXTERNAL recupera os contornos externos
    //CV_CHAIN_APPROX_SIMPLE comprime os segmentos horizontais, verticais e diagonais,
    //deixando apenas os seus pontos finais - é um método de aproximação
    findContours(dist_8u, contours, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_SIMPLE);

    //Retorna uma matriz zeros do tamanho e do tipo especificado (CV_32SC1)
    Mat markers = Mat::zeros(dist.size(), CV_32SC1);

    //Desenha os marcadores no primeiro plano
    for (size_t i = 0; i < contours.size(); i++)
        drawContours(markers, contours, static_cast<int>(i), Scalar::all(static_cast<int>(i)+1), -1);

    //Desenha os marcadores de fundo
    circle(markers, Point(5,5), 3, CV_RGB(255,255,255), -1);

    //Exibe os marcadores que já foram desenhados
  //  imshow("Marcadores", markers*10000);

    //Retorna outra matriz zeros do tamanho e do tipo especificado (CV_8UC1)
    Mat mark = Mat::zeros(markers.size(), CV_8UC1);

    //Converte a imagem para CV_8UC1
    markers.convertTo(mark, CV_8UC1);

    //Inverte os bits do array mark
    bitwise_not(mark, mark);

    //Um vetor para armazenar as cores randomicamente é criado
    //Logo em seguida o vetor de cores é iniciado com as cores
    vector<Vec3b> cores;
    for (size_t i = 0; i < contours.size(); i++)
    {
        int b = theRNG().uniform(0, 255);
        int g = theRNG().uniform(0, 255);
        int r = theRNG().uniform(0, 255);
        cores.push_back(Vec3b((uchar)b, (uchar)g, (uchar)r));
    }

    //Cria a imagem de resultado
    Mat dst = Mat::zeros(markers.size(), CV_8UC3);

    //Preenche (rotula) os objetos marcados com as cores aletórias existentes no vetor 'colors'
    for (int i = 0; i < markers.rows; i++)
    {
        for (int j = 0; j < markers.cols; j++)
        {
            int index = markers.at<int>(i,j);
            if (index > 0 && index <= static_cast<int>(contours.size()))
                dst.at<Vec3b>(i,j) = cores[index-1];
            else
                dst.at<Vec3b>(i,j) = Vec3b(0,0,0);
        }
    }

    //Exibe a imagem final
    imshow("Resultado final", dst);

  /// Get the moments
  vector<Moments> mu(contours.size() );
  for( int i = 0; i < contours.size(); i++ )
     { mu[i] = moments( contours[i], false ); }

  ///  Get the mass centers:
  vector<Point2f> mc( contours.size() );
  for( int i = 0; i < contours.size(); i++ )
     { mc[i] = Point2f( mu[i].m10/mu[i].m00 , mu[i].m01/mu[i].m00 ); }

for( int i = 0; i < contours.size(); i++ )
     {
       cout << "Area aproximada do bolor: " <<(contourArea(contours[i]) * (0.026458 * 0.026458)) << "cm^2"<< endl;

       Scalar color = Scalar( rng.uniform(0, 255), rng.uniform(0,255), rng.uniform(0,255) );
       drawContours(dst, contours, i, color, 2, 8, hierarchy, 0, Point() );
       circle(dst, mc[i], 4, color, -1, 8, 0 );
     }

    waitKey(0);
    return 0;
}
