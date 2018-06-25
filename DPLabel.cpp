#include "DPLabel.h"



DPLabel::DPLabel()
{
}


DPLabel::~DPLabel()
{
}

void DPLabel::simplifyLabels(std::vector<LabelProp> labelProps
        , std::vector<int> &newLabelTable
        , std::vector < LabelProp > &newLabelProps
)
{
        for (int i = 0; i < labelProps.size(); i++)
        {
                LabelProp rootLabel;
                int nRootIdx = i;
                rootLabel = labelProps[i];

                // Dig in
                while (1)
                {
                        if (rootLabel.isRoot()) {
                                break;
                        }

                        nRootIdx = rootLabel.nNextLabelIdx;
                        rootLabel = labelProps[nRootIdx];
                }


                labelProps[nRootIdx].combine(labelProps[i]);
                newLabelTable.push_back(labelProps[nRootIdx].nClass);
        }

        for (int i = 0; i < labelProps.size(); i++)
        {
                if (labelProps[i].isRoot())
                        newLabelProps.push_back(labelProps[i]);
        }

}

cv::Mat DPLabel::getLabelImage()
{
        if (!matLabel_second.empty())
                return matLabel_second;

        matLabel_first.copyTo(matLabel_second);
        for (int i = 0; i < matLabel_first.cols * matLabel_first.rows; i++)
        {
                unsigned short val = matLabel_first.at<unsigned short>(i);

                if(val > 0 && val < m_labelTable.size())
                        matLabel_second.at<unsigned short>(i) = m_labelTable[val - 1];
        }

        return matLabel_second;
}

std::vector<cv::Rect> DPLabel::getBoundingBoxes()
{
        std::vector<cv::Rect> boxes;

        for (int i = 0; i < m_labelProps.size(); i++)
        {
                boxes.push_back(cv::Rect(m_labelProps[i].minPt, m_labelProps[i].maxPt + cv::Point(1,1)));
        }


        return boxes;
}

int DPLabel::run(cv::Mat matBinary)
{
        int nWidth = matBinary.cols;
        int nHeight = matBinary.rows;
        matLabel_second = cv::Mat();

        cv::Mat matLabel = cv::Mat::zeros(matBinary.rows, matBinary.cols, CV_16UC1);
        unsigned short *label_rows[2];
        uchar *bin_row;

        unsigned int y_pre;
        unsigned int x_pre;
        unsigned int x_pos;
        int nLabels = 1;

        // Clear label list
        std::vector<LabelProp> labelProps;
        labelProps.clear();
        m_labelProps.clear();
        m_labelTable.clear();

        for (int y = 0; y < nHeight; y++)
        {
                // Exceptions handling
                y_pre = y - 1 < 0 ? 0 : y - 1;

                // Read the rows
                label_rows[0] = matLabel.ptr<unsigned short>(y_pre);
                label_rows[1] = matLabel.ptr<unsigned short>(y);
                bin_row = matBinary.ptr(y);

                for (int x = 0; x < nWidth; x++)
                {
                        // If not foreground, skip
                        if ( bin_row[x] == 0)
                                continue;

                        // Exceptions handling
                        x_pre = x - 1 < 0 ? 0 : x - 1;
                        x_pos = x + 1 >= nWidth -1 ? nWidth - 1 : x + 1;

                        // Read the values
                        uchar uValue[4] = {0,0,0,0};
                        unsigned short *cur_label = &label_rows[1][x];
                        uValue[0] = label_rows[1][x_pre]; // Check Left
                        uValue[1] = label_rows[0][x_pre]; // Check Top Left
                        uValue[2] = label_rows[0][x]; // Check Top
                        uValue[3] = label_rows[0][x_pos]; // Check Top Right

                        // If neighbor is empty, set new label and end
                        if (uValue[0] + uValue[1] + uValue[2] + uValue[3] == 0)
                        {
                                *cur_label = nLabels;
                                labelProps.push_back(LabelProp(nLabels,cv::Point(x,y)));
                                nLabels++;
                                continue;
                        }

                        // Checking neighbors
                        for (int i = 0; i < 4; i++)
                        {
                                // Skip if empty
                                if (uValue[i] <= 0)
                                        continue;

                                // If different
                                if (*cur_label != uValue[i])
                                {
                                        // If current label isnt empty
                                        if (*cur_label > 0)	{
                                                labelProps[uValue[i] - 1 ].setNext(*cur_label - 1);
                                        }
                                        // If current label empty
                                        else {
                                                *cur_label = uValue[i];
                                                labelProps[*cur_label - 1].update(cv::Point(x,y));
                                        }
                                }
                        }

                }

        }

        matLabel.copyTo(matLabel_first);
        simplifyLabels(labelProps, m_labelTable, m_labelProps);

        return 0;
}
