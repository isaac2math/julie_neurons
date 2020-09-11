#include "prelu.hpp"


julie::nn::func::PReLU::PReLU(const std::shared_ptr<op::Variable> & t_ptr, const std::shared_ptr<op::Variable> & alpha)
    :
    op::Function {},
    m_relu {std::make_unique<la::PReLU<double>>()},
    m_diff {}
{
    // double var = static_cast<double>(100) / w_mat.shape().size();
    // w_mat.gaussian_random(0, var);

    this->m_inputs.push_back(t_ptr);
    this->m_inputs.push_back(alpha);

    t_ptr->add_receiver(this);
    alpha->add_receiver(this);

    this->m_output = std::make_shared<var::Tensor<double>> ();
    this->m_output->set_provider(this);
}

julie::nn::func::PReLU::PReLU(const PReLU & other)
    :
    op::Function {other},
    m_relu {std::make_unique<la::PReLU<double>>(*(other.m_relu))}
{}

julie::nn::func::PReLU::PReLU(PReLU && other)
    :
    op::Function {other},
    m_relu {std::move(other.m_relu)}
{}

julie::nn::func::PReLU & julie::nn::func::PReLU::operator = (const PReLU & other)
{
    op::Function::operator = (other);
    this->m_relu = std::make_unique<la::PReLU<double>>(*(other.m_relu));

    return *this;
}

julie::nn::func::PReLU & julie::nn::func::PReLU::operator = (PReLU && other)
{
    op::Function::operator = (other);
    this->m_relu = std::move(other.m_relu);

    return *this;
}

void julie::nn::func::PReLU::forward()
{
    var::Tensor<double> *input_ptr = dynamic_cast<var::Tensor<double>*>(this->m_inputs[0].get());
    std::shared_ptr<la::DMatrix<double>> t_mat_ptr = input_ptr->val();

    var::Scalar<double> *alpha_ptr = dynamic_cast<var::Scalar<double>*>(this->m_inputs[1].get());
    std::shared_ptr<double> alpha_val_ptr = alpha_ptr->val();

    var::Tensor<double> *output_ptr = dynamic_cast<var::Tensor<double>*>(this->m_output.get());

    la::DMatrix<double> output_mat;

    if (input_ptr->needs_grad())
    {
        this->m_relu->operator()(output_mat, this->m_diff, this->m_alpha_diff, *t_mat_ptr, *alpha_val_ptr);
    }
    else
    {
        this->m_relu->operator()(output_mat, *t_mat_ptr, *alpha_val_ptr);
    }

    output_ptr->val(std::move(output_mat));
}

void julie::nn::func::PReLU::backward()
{
    var::Tensor<double> *output_ptr = dynamic_cast<var::Tensor<double>*>(this->m_output.get());
    std::shared_ptr<la::DMatrix<double>> out_grad = output_ptr->grad();

    var::Tensor<double> *input_ptr = dynamic_cast<var::Tensor<double>*>(this->m_inputs[0].get());
    var::Scalar<double> *alpha_ptr = dynamic_cast<var::Scalar<double>*>(this->m_inputs[1].get());

    std::shared_ptr<la::DMatrix<double>> t_mat_ptr = input_ptr->val();

    // std::cout << *out_grad << std::endl;

    if (input_ptr->needs_grad())
    {
        // Do chain rule for the input
        input_ptr->grad(la::multiply(this->m_diff, *out_grad));
    }

    if (alpha_ptr->needs_grad())
    {
        // Do chain rule for the alpha
        alpha_ptr->grad(la::dot_product(this->m_alpha_diff, *out_grad) / this->m_alpha_diff.shape().size()); 
    }
}
