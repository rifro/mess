#pragma once
#include "rdv_types.h"

namespace Bocari
{
    /**
     * @brief A simple 3x3 matrix for Kalman filter calculations.
     * The layout is row-major.
     */
    struct Matrix3x3
    {
        float m[3][3];
    };

    /**
     * @brief Represents a standard orthogonal frame composed of three axes.
     */
    struct OrthogonalFrame
    {
        Vec3f m_xAxis;
        Vec3f m_yAxis;
        Vec3f m_zAxis;
    };

    /**
     * @brief Holds the state for a single axis being tracked by the Kalman filter.
     * The state consists of the estimated direction and the uncertainty of that estimate.
     */
    struct KalmanAxisState
    {
        Vec3f m_stateEstimate;      // x_hat: The estimated axis direction vector.
        Matrix3x3 m_errorCovariance;  // P: The error covariance matrix.
    };

    /**
     * @brief A simple Kalman filter to stabilize a single 3D vector (axis) over time.
     * To stabilize a full orthogonal frame, three instances of this filter can be used,
     * one for each principal axis.
     */
    class KalmanFilter
    {
    public:
        KalmanFilter();

        /**
         * @brief Initializes the filter's state.
         * @param initialState The initial estimate for the axis.
         * @param initialUncertainty A scalar representing the initial uncertainty of the estimate.
         */
        void initialize(const Vec3f& initialState, float initialUncertainty);

        /**
         * @brief Predicts the next state based on the internal process model.
         * For stabilizing axes, this is often a simple identity prediction.
         */
        void predict();

        /**
         * @brief Updates the state estimate with a new measurement.
         * @param measurement The new measured axis from the RDV voter.
         * @param measurementNoise A scalar representing the uncertainty of the measurement.
         */
        void update(const Vec3f& measurement, float measurementNoise);

        /**
         * @brief Returns the current stable estimate of the axis.
         */
        Vec3f getStableAxis() const;

    private:
        KalmanAxisState m_state;

        // Q: Process noise covariance matrix, represents the uncertainty in the model.
        Matrix3x3 m_processNoiseCovariance;
    };

} // namespace Bocari
